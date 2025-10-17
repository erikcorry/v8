// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer.h"

#include "src/common/ptr-compr-inl.h"
#include "src/heap/visit-object.h"
#include "src/objects/slots-inl.h"
#include "src/objects/slots.h"

namespace v8 {
namespace internal {

FastSerializer::~FastSerializer() {}

FastSerializer::FastSerializer(Isolate* isolate,
                               Snapshot::SerializerFlags flags)
    : zone_(&allocator_, "FastSerializer"),
      queue_(&zone_),
      lab_liveness_map_(&zone_),
      lab_map_(&zone_),
      new_locations_(&zone_),
      all_labs_(&zone_),
      cage_base_(isolate),
      external_reference_encoder_(isolate),
      flags_(flags),
      previous_serializers_(&zone_),
      snapshot_(new FastSnapshot()) {
  // Nothing points at the roots, so the location is not important.  We use the
  // zero location for this lab.
  Address location = 0;
  roots_lab_ = zone_.New<LinearAllocationBuffer>(
      &zone_, next_lab_index_++, kIgnoreAllocationSpace, AddressSpace::kRoots,
      location, location);
  for (int space = 0; space < kNumberOfAddressSpaces; space++) {
    address_space_fullnesses_[space] = 0;
  }
  for (int space = FIRST_SPACE; space <= LAST_SPACE; space++) {
    lab_per_space_[space] = nullptr;
  }
}

int FastSerializer::next_lab_index_ = 0;

void FastSerializer::Serialize() {
  DisallowGarbageCollection no_gc;
  // Make sure things get linked in.
  ProcessQueue();
  // TODO: The below is from ReadonlySerializer.
  /*
  ReadOnlyHeapImageSerializer::Serialize(isolate(), &sink_,
                                         GetUnmappedRegions(isolate()));

  ReadOnlyHeapObjectIterator it(isolate()->read_only_heap());
  for (Tagged<HeapObject> o = it.Next(); !o.is_null(); o = it.Next()) {
    CheckRehashability(o);
    if (v8_flags.serialization_statistics) {
      CountAllocation(o->map(), o->Size(), SnapshotSpace::kReadOnlyHeap);
    }
  }
  */
}
AddressSpace FastSerializer::GetAddressSpace(Tagged<HeapObject> object) {
  if (flags_ & Snapshot::kAllInOneSpace) {
    return kSharedSpace;
  }
#ifndef V8_COMPRESS_POINTERS
  return kUncompressed;
#else
  // Map is always in the regular cage.
  Tagged<Map> map = object->map(cage_base());
  if (object.IsInMainCageBase()) {
    return kMainCage;
  } else if (InstanceTypeChecker::IsTrustedObject(map)) {
    return kTrustedCage;
  } else if (InstanceTypeChecker::IsInstructionStream(map)) {
    return kCodeSpace;
  } else {
    UNREACHABLE();
  }
#endif
}

AllocationSpace FastSerializer::GetAllocationSpace(Tagged<HeapObject> object) {
  MemoryChunkMetadata* metadata =
      MemoryChunkMetadata::FromHeapObject(isolate_, object);
  return metadata->owner()->identity();
}

bool FastSerializer::IsMarked(Tagged<HeapObject> object) {
  if (LocalIsMarked(object)) return true;
  for (auto it : previous_serializers_) {
    if (it->LocalIsMarked(object)) return true;
  }
  return false;
}

bool FastSerializer::LocalIsMarked(Tagged<HeapObject> object) {
  Address lab_start = RoundDown(object->address(), kRegularPageSize);
  uint64_t*& bitmap = lab_liveness_map_[lab_start];
  if (bitmap == nullptr) return false;
  constexpr int kWordSize = sizeof(uint32_t);  // Words on a compressed heap.
  size_t byte_offset = object->address() - lab_start;
  size_t index = byte_offset / (64 * kWordSize);
  int bit_number = (byte_offset / kWordSize) & 63;
  return (bitmap[index] & (uint64_t{1} << bit_number)) != 0;
}

void FastSerializer::Mark(Tagged<HeapObject> object, size_t size) {
  Address lab_start = RoundDown(object->address(), kRegularPageSize);
  auto it = lab_liveness_map_.find(lab_start);
  uint64_t* bitmap = (it == lab_liveness_map_.end()) ? nullptr : it->second;
  if (bitmap == nullptr) {
    // We don't yet have a liveness map for this lab, so we allocate one.
    // Each 32 bit word is a bit in the map so we divide the size with 32.
    bitmap = reinterpret_cast<uint64_t*>(
        zone_.Allocate<uint64_t>(kRegularPageSize / 32));
  }
  size_t start_offset = object->address() - lab_start;
  size_t end_offset = object->address() + size - lab_start;
  end_offset = std::max(end_offset, size_t{kRegularPageSize});
  constexpr int kWordSize = sizeof(uint32_t);  // Words on a compressed heap.
  // Mark all words in the object. TODO(erikcorry): Do this more efficiently.
  for (size_t o = start_offset; o < end_offset; o += sizeof(uint32_t)) {
    size_t index = o / (64 * kWordSize);
    int bit_number = (o / kWordSize) & 63;
    bitmap[index] |= uint64_t{1} << bit_number;
  }
}

std::unique_ptr<FastSnapshot> FastSerializer::Run() {
  return std::move(fast_snapshot_);
}

// The roots are conceptually in a synthetic lab that contains each root slot
// in order.  We encode the roots in the same way as slots in other labs.
void FastSerializer::VisitRootPointers(
    Root root,  // An enum - see ROOT_ID_LIST.
    const char* description,
    FullObjectSlot start,  // From src/objects/slots.h.
    FullObjectSlot end) {
  for (FullObjectSlot current = start; current < end; ++current) {
    size_t offset = roots_lab_->highest();
    roots_lab_->Expand(offset, offset + sizeof(Address));
    snapshot_->root_lab_data_.push_back((*current).ptr());
    VisitSlot(roots_lab_, offset, current);
  }
}

FastSerializer::LabAndOffset FastSerializer::NewLocation(Address start) {
  auto it = new_locations_.find(start);
  if (it != new_locations_.end()) {
    return it->second;
  }
  for (auto prev : previous_serializers_) {
    auto it2 = prev->new_locations_.find(start);
    if (it2 != prev->new_locations_.end()) {
      return it2->second;
    }
  }
  UNREACHABLE();
}

// Called on slots.
// Marks and queues any new objects discovered that are part of the snapshot.
// The caller has already used cage bases etc. to determine the real location
// of any compressed slots, and the uncompressed pointed-to Address is passed
// to us.
// If we are reallocating the objects in a new location then it is called on
// the location of the new slot.
// In this case it will rewrite any slots that point to HeapObjects to point to
// the new location.
template <typename Slot>
void FastSerializer::VisitSlot(
    LinearAllocationBuffer* slot_lab,  // Lab containing slot.
    size_t slot_offset,                // Position of slot in slot_lab.
    Slot maybe_smi) {
  // Don't use operator * here because it will use the location of the slot
  // to determine the cage base, but we might have memcpyed the object to a
  // lab by this time.  The load method uses the exact templated type of Slot
  // to work out which cage, if any, the contents should be interpreted
  // relative to.
  typename Slot::TObject slot_contents = maybe_smi.load(isolate_);
  Tagged<HeapObject> heap_object;
  // We don't need to do anything for Smis.  GetHeapObject returns true for
  // both weak and strong heap object slots.
  if (!slot_contents.GetHeapObject(&heap_object)) return;
  // If the slot is a cleared weak slot then we don't need to do anything.
  // TODO: What if it's a 64 bit slot and only the bottom 32 bits are cleared?
  if (Slot::kCanBeWeak && heap_object.IsCleared()) return;
  Address start = heap_object.address();
  if ((flags_ & Snapshot::kUseIsolateMemory) != 0) {
    size_t size = heap_object->Size();
    if (!IsMarked(heap_object)) {
      Mark(heap_object, size);
      queue_.push_back(heap_object);
    }
    LinearAllocationBuffer* lab;
    if (LocalIsMarked(heap_object)) {
      // The object belongs to the current serializer.
      // The labs just record the real location of the objects.
      lab = GetOrCreateLabForFixedLocation(heap_object);
      // TODO: We need to check whether the same fixed-location lab is in more
      // than one stacked serializer - we probably can't handle that.
    } else {
      // The object is part of a previous serializer, so it's allocated in one
      // of that serializer's labs.
      lab = GetLabForFixedLocation(heap_object);
    }
    if (lab->index() != slot_lab->index()) {
      snapshot_->AddRelocation(slot_lab->index(), lab->index(), slot_offset,
                               true);
    }
    // Mark this object's range as part of the lab - may be a no-op if that
    // already happened.
    lab->Expand(start, start + size);
  } else {
    // We are constructing artificial labs for the snapshot.
    // Check if the slot points at an object that wasn't until now
    // part of the snapshot.
    LinearAllocationBuffer* lab = nullptr;
    size_t offset = 0;
    if (!IsMarked(heap_object)) {
      // We need to find a lab
      size_t size = heap_object->Size();
      // Make the object gray/black.
      Mark(heap_object, size);
      // The objects are reallocated in tightly packed labs that we create
      // in the serializer for use by the deserializer.
      lab = GetOrCreatePackedLab(heap_object, size);
      offset = lab->highest();
      lab->Expand(offset, offset + size);
      // We may overwrite slots in the object later, but for now we copy
      // everything in the newly discovered object into the allocation.
      memcpy(lab->BackingAt(offset), reinterpret_cast<void*>(start), size);
      new_locations_[start] = {lab, offset};

      // Make the object gray and queue for processing.
      Tagged<HeapObject> new_location(
          reinterpret_cast<Address>(lab->BackingAt(offset)));
      queue_.push_back(new_location);
    } else {
      auto location = NewLocation(start);
      lab = location.lab;
      offset = location.offset;
    }

    // Update the slot to point to the new location.  The object has already
    // been memcpy'ed into the lab, so we are doing the relocation there.
    bool is_compressed;
    if constexpr (requires { Slot::TCompressionScheme; }) {
      is_compressed = true;
      // We just want to store the new offset to the compressed 32 bit slot.
      // We make a fake pointer, which is at a given offset in the cage of the
      // slot.
      Tagged<Object> new_location(
          Slot::TCompressionScheme::DecompressTagged::base() + offset);
      // This compressing store just writes the last 32 bits of the fake
      // pointer, ie the offset.  The store can assert that the location is in
      // the current cage, which is why we have to add the correct top 32 bits
      // above.
      maybe_smi.store(new_location);
    } else {
      // Make a fake pointer to an object at a 32 bit offset that corresponds
      // to the offset.  The high 32 bits are zero, but that will be fixed by
      // the relocation.
      Tagged<Object> new_location(offset);
      maybe_smi.store(new_location);
      is_compressed = false;
    }
    // Store a relocation that (if necessary) will change the pointer to the
    // correct location on deserialization.
    snapshot_->AddRelocation(slot_lab->index(), lab->index(), slot_offset,
                             is_compressed);
  }
}

LinearAllocationBuffer* FastSerializer::GetLabForFixedLocation(
    Tagged<HeapObject> object) {
  DCHECK((flags_ & Snapshot::kUseIsolateMemory) != 0);
  DCHECK((flags_ & Snapshot::kAllInOneSpace) == 0);
  Address rounded_address = RoundDown(object.address(), kRegularPageSize);

  LinearAllocationBuffer*& lab = lab_map_[rounded_address];
  if (lab != nullptr) return lab;
  for (auto it : previous_serializers_) {
    lab = it->lab_map_[rounded_address];
    if (lab != nullptr) return lab;
  }
  UNREACHABLE();
}

LinearAllocationBuffer* FastSerializer::GetOrCreateLabForFixedLocation(
    Tagged<HeapObject> object) {
  DCHECK((flags_ & Snapshot::kUseIsolateMemory) != 0);
  DCHECK((flags_ & Snapshot::kAllInOneSpace) == 0);
  AddressSpace address_space = GetAddressSpace(object);
  Address rounded_address = RoundDown(object.address(), kRegularPageSize);

  AllocationSpace space = GetAllocationSpace(object);

  LinearAllocationBuffer*& lab = lab_map_[rounded_address];
  if (lab == nullptr) {
    // Updates lab_map_ because it's a reference.
    lab = zone_.New<LinearAllocationBuffer>(&zone_, next_lab_index_++, space,
                                            address_space, object.address(),
                                            object.address());
    all_labs_.push_back(lab);
  }
  return lab;
}

LinearAllocationBuffer* FastSerializer::GetOrCreatePackedLab(
    Tagged<HeapObject> object, size_t object_size) {
  AllocationSpace allocation_space = GetAllocationSpace(object);
  LinearAllocationBuffer* lab = lab_per_space_[allocation_space];
  while (true) {
    if (!lab) {
      AddressSpace address_space = GetAddressSpace(object);
      size_t fullness = address_space_fullnesses_[address_space];
      address_space_fullnesses_[address_space] += kRegularPageSize;
      lab = zone_.New<LinearAllocationBuffer>(
          &zone_, next_lab_index_++, allocation_space, address_space, fullness);
      lab_per_space_[allocation_space] = lab;
      all_labs_.push_back(lab);
    }
    size_t used = lab->highest() - lab->start();
    if (used + object_size <= kRegularPageSize) {
      // Enough space.
      return lab;
    }
    lab = nullptr;
  }
}

FastSnapshot* SerializeIsolate() {
  // TODO.
  return nullptr;
}

void FastSerializer::ProcessQueue() {
  while (!queue_empty()) {
    Tagged<HeapObject> object = queue_.back();
    queue_.pop_back();
    size_t dest_offset;
    LinearAllocationBuffer* dest_lab;
    if ((flags_ & Snapshot::kUseIsolateMemory) == 0) {
      // This function is always given the original address of the object, but
      // we also need access to the fields in the reallocated (serialized)
      // object.
      auto lab_and_offset = new_locations_[object.address()];
      dest_lab = lab_and_offset.lab;
      dest_offset = lab_and_offset.offset;
      // Point at the new location of the object so the visitor can rewrite
      // slots.
      object = Tagged<HeapObject>(dest_lab->start() + dest_offset);
    } else {
      // Using isolate memory for the lab, so the object doesn't move.
      Address dest_address = object.address();
      Address rounded_address = RoundDown(dest_address, kRegularPageSize);
      dest_lab = lab_map_[rounded_address];
      dest_offset = dest_address - rounded_address;
    }
    ObjectSerializer serializer(this, object, dest_lab, dest_offset);
    serializer.SerializeObject();
  }
}

void FastSerializer::ObjectSerializer::SerializeObject() {
  CHECK(serializer_->IsMarked(object_));
  // TODO: Do we need to avoid the visitors processing weakness.  Search for
  // "Descriptor arrays have complex element weakness".
  // TODO: WTF UnlinkWeakNextScope unlink_weak_next(isolate()->heap(), object_);
  // Iterate references.  This will call some of the virtuals on the
  // ObjectSerializer.
  VisitObject(serializer_->isolate(), object_, this);
  // Nothing to do for the data payload.
}

void FastSerializer::ObjectSerializer::VisitPointers(Tagged<HeapObject> host,
                                                     CompressedObjectSlot start,
                                                     CompressedObjectSlot end) {
  VisitPointers(host, MaybeObjectSlot(start), MaybeObjectSlot(end));
}

void FastSerializer::ObjectSerializer::VisitPointers(
    Tagged<HeapObject> host, CompressedMaybeObjectSlot start,
    CompressedMaybeObjectSlot end) {
  Address base_address = object_.address();
  for (CompressedMaybeObjectSlot slot = start; slot <= end; slot++) {
    size_t offset = slot.address() - base_address;
    serializer_->VisitSlot(dest_lab_, offset, slot);
  }
}

void FastSerializer::ObjectSerializer::VisitInstructionStreamPointer(
    Tagged<Code> host, InstructionStreamSlot slot) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitCodeTarget(
    Tagged<InstructionStream>, RelocInfo*) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitEmbeddedPointer(
    Tagged<InstructionStream>, RelocInfo*) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitExternalReference(
    Tagged<InstructionStream>, RelocInfo*) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitInternalReference(
    Tagged<InstructionStream>, RelocInfo*) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitExternalPointer(
    Tagged<HeapObject>, ExternalPointerSlot) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitIndirectPointer(
    Tagged<HeapObject>, IndirectPointerSlot, IndirectPointerMode) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitProtectedPointer(
    Tagged<TrustedObject>,
    OffHeapCompressedObjectSlot<V8HeapCompressionSchemeImpl<TrustedCage>>) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitProtectedPointer(
    Tagged<TrustedObject>, OffHeapCompressedMaybeObjectSlot<
                               V8HeapCompressionSchemeImpl<TrustedCage>>) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitTrustedPointerTableEntry(
    Tagged<HeapObject>, IndirectPointerSlot) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitJSDispatchTableEntry(
    Tagged<HeapObject>,
    v8::base::StrongAlias<JSDispatchHandleAliasTag, unsigned int>) {
  UNREACHABLE();
}

}  // namespace internal
}  // namespace v8
