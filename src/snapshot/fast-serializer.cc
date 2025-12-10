// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer.h"

#include "src/common/ptr-compr-inl.h"
#include "src/heap/heap-visitor-inl.h"
#include "src/heap/heap-visitor.h"
#include "src/heap/visit-object.h"
#include "src/objects/compressed-slots-inl.h"
#include "src/objects/compressed-slots.h"
#include "src/objects/slots-inl.h"
#include "src/objects/slots.h"

namespace v8 {
namespace internal {

FastSerializer::~FastSerializer() {}

FastSerializer::FastSerializer(Isolate* isolate,
                               Snapshot::SerializerFlags flags)
    : isolate_(isolate),
      zone_(&allocator_, "FastSerializer"),
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
      location);
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
  FinalizeSerialization();
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
#ifndef V8_COMPRESS_POINTERS
  return kUncompressed;
#else
  if (flags_ & Snapshot::kAllInOneSpace) {
    // If the serializer is moving all reachable objects into one space then
    // that's the read-only space in the main cage.
    return kMainCage;
  }
  // Map is always in the regular cage.
  Tagged<Map> map = object->map(cage_base());
  if (object.IsInMainCageBase()) {
    return kMainCage;
  } else if (object.IsInTrustedCageBase()) {
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
  BaseSpace* owner = metadata->owner();
  if (!owner) return RO_SPACE;
  return owner->identity();
}

bool FastSerializer::IsMarked(Address address) {
  if (LocalIsMarked(address)) return true;
  for (auto it : previous_serializers_) {
    if (it->LocalIsMarked(address)) return true;
  }
  return false;
}

bool FastSerializer::LocalIsMarked(Address address) {
  Address lab_start = RoundDown(address, kRegularPageSize);
  uint64_t*& bitmap = lab_liveness_map_[lab_start];
  if (bitmap == nullptr) return false;
  constexpr int kWordSize = sizeof(uint32_t);  // Words on a compressed heap.
  size_t byte_offset = address - lab_start;
  size_t index = byte_offset / (64 * kWordSize);
  int bit_number = (byte_offset / kWordSize) & 63;
  return (bitmap[index] & (uint64_t{1} << bit_number)) != 0;
}

void FastSerializer::Mark(Address address, size_t size) {
  if (0x39f50003f800 <= address && address < 0x39f500040000) {
    printf("Mark from %p-%p\n", (void*)address, (char*)address + size);
  }
  Address lab_start = RoundDown(address, kRegularPageSize);
  auto it = lab_liveness_map_.find(lab_start);
  uint64_t* bitmap = (it == lab_liveness_map_.end()) ? nullptr : it->second;
  if (bitmap == nullptr) {
    // We don't yet have a liveness map for this lab, so we allocate one.
    // Each 32 bit word is a bit in the map so we divide the size with 32.
    auto length = kRegularPageSize / 32;
    bitmap = reinterpret_cast<uint64_t*>(zone_.Allocate<uint64_t>(length));
    lab_liveness_map_[lab_start] = bitmap;
    memset(bitmap, 0, length);
  }
  size_t start_offset = address - lab_start;
  size_t end_offset = address + size - lab_start;
  end_offset = std::min(end_offset, size_t{kRegularPageSize});
  constexpr int kWordSize = sizeof(uint32_t);  // Words on a compressed heap.
  // Mark all words in the object. TODO(erikcorry): Do this more efficiently.
  for (size_t o = start_offset; o < end_offset; o += kWordSize) {
    size_t index = (o / kWordSize) / 64;
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
    if (is_read_only()) {
      Tagged<Object> value = current.load(isolate_);
      if (!value.IsSmi()) {
        AllocationSpace allocation_space =
            GetAllocationSpace(Cast<HeapObject>(value));
        if (allocation_space != RO_SPACE) {
          // For the read-only serializer we skip other roots.
          return;
        }
      }
    }
    size_t offset = roots_lab_->highest();
    MarkTableEntry(roots_lab_, offset, sizeof(Address));
    snapshot_->root_lab_data_.push_back((*current).ptr());
    FullObjectSlot new_slot(
        reinterpret_cast<Address>(roots_lab_->BackingAt(offset)));
    VisitSlot(roots_lab_, offset, current, new_slot);
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
    Slot old_slot,                     // Slot in original object.
    Slot new_slot) {                   // Slot in object in lab.
  // Don't use operator * here because it will use the location of the slot
  // to determine the cage base, but we might have memcpyed the object to a
  // lab by this time.  The load method uses the exact templated type of Slot
  // to work out which cage, if any, the contents should be interpreted
  // relative to.
  typename Slot::TObject slot_contents = old_slot.load(isolate_);
  Tagged<HeapObject> heap_object;
  // We don't need to do anything for Smis.  GetHeapObject returns true for
  // both weak and strong heap object slots.
  if (!slot_contents.GetHeapObject(&heap_object)) return;
  // If the slot is a cleared weak slot then we don't need to do anything.
  // TODO: What if it's a 64 bit slot and only the bottom 32 bits are cleared?
  if (Slot::kCanBeWeak && heap_object.IsCleared()) return;
  LabAndOffset location = FoundObject(heap_object);
  bool is_compressed = true;
  if ((flags_ & Snapshot::kUseIsolateMemory) == 0) {
    // Update the slot to point to the new location.  The object has already
    // been memcpy'ed into the lab, so we are doing the relocation there.
    if constexpr (requires { typename Slot::TCompressionScheme; }) {
      // We just want to store the new offset to the compressed 32 bit slot.
      // We make a fake pointer, which is at a given offset in the cage of the
      // slot.
      Tagged<Object> new_location(Slot::TCompressionScheme::DecompressTagged(
          static_cast<uint32_t>(location.offset) + kHeapObjectTag));
      // This compressing store just writes the last 32 bits of the fake
      // pointer, ie the offset.  The store can assert that the location is in
      // the current cage, which is why we have to add the correct top 32 bits
      // above.
      new_slot.store(new_location);
    } else {
      // Make a fake pointer to an object at a 32 bit offset that corresponds
      // to the offset.  The high 32 bits are zero, but that will be fixed by
      // the relocation.
      Tagged<Object> new_location(location.offset + kHeapObjectTag);
      new_slot.store(new_location);
      is_compressed = false;
    }
  }
  // Store a relocation that (if necessary) will change the pointer to the
  // correct location on deserialization.
  snapshot_->AddRelocation(slot_lab->index(), location.lab->index(),
                           slot_offset, is_compressed);
}

FastSerializer::LabAndOffset FastSerializer::FoundObject(
    Tagged<HeapObject> heap_object) {
  Address start = heap_object.address();
  if ((flags_ & Snapshot::kUseIsolateMemory) != 0) {
    size_t size = heap_object->Size();
    if (!IsMarked(heap_object->address())) {
      Mark(heap_object->address(), size);
      queue_.push_back(heap_object);
    }
    LinearAllocationBuffer* lab;
    if (LocalIsMarked(heap_object->address())) {
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
    // Mark this object's range as part of the lab - may be a no-op if that
    // already happened.
    lab->Expand(start, start + size);
    return {lab, start};
  } else {
    // We are constructing artificial labs for the snapshot.
    // Check if the slot points at an object that wasn't until now
    // part of the snapshot.
    if (!IsMarked(heap_object->address())) {
      // We need to find a lab
      size_t size = heap_object->Size();
      // Make the object gray/black.
      Mark(heap_object->address(), size);
      // The objects are reallocated in tightly packed labs that we create
      // in the serializer for use by the deserializer.
      LinearAllocationBuffer* lab = GetOrCreatePackedLab(heap_object, size);
      size_t offset = lab->highest();
      lab->Expand(offset, offset + size);
      // We may overwrite slots in the object later, but for now we copy
      // everything in the newly discovered object into the allocation.
      memcpy(lab->BackingAt(offset), reinterpret_cast<void*>(start), size);
      new_locations_[start] = {lab, offset};
      // Make the object gray and queue for processing.
      queue_.push_back(heap_object);
    }
    return NewLocation(start);
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
  AllocationSpace allocation_space = GetAllocationSpace(object);
  return GetOrCreateLabForFixedLocation(object->address(), address_space,
                                        allocation_space);
}

LinearAllocationBuffer* FastSerializer::GetOrCreateLabForFixedLocation(
    Address address, AddressSpace address_space,
    AllocationSpace allocation_space) {
  Address rounded_address = RoundDown(address, kRegularPageSize);

  LinearAllocationBuffer*& lab = lab_map_[rounded_address];
  if (lab == nullptr) {
    // Updates lab_map_ because it's a reference.
    lab = zone_.New<LinearAllocationBuffer>(&zone_, next_lab_index_++,
                                            allocation_space, address_space,
                                            address, address);
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
          &zone_, next_lab_index_++, OLD_SPACE, address_space, fullness);
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

void FastSerializer::FinalizeSerialization() {
  while (!queue_empty()) {
    Tagged<HeapObject> object = queue_.back();
    Tagged<HeapObject> new_object = object;
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
      new_object = Tagged<HeapObject>(reinterpret_cast<Address>(
          dest_lab->BackingAt(dest_offset) + kHeapObjectTag));
    } else {
      // Using isolate memory for the lab, so the object doesn't move.
      Address dest_address = object.address();
      Address rounded_address = RoundDown(dest_address, kRegularPageSize);
      dest_lab = lab_map_[rounded_address];
      dest_offset = dest_address - rounded_address;
    }
    ObjectSerializer serializer(this, object, new_object, dest_lab,
                                dest_offset);
    serializer.SerializeObject();
  }
}

FastSerializer::ObjectSerializer::ObjectSerializer(
    FastSerializer* serializer, Tagged<HeapObject> old_object,
    Tagged<HeapObject> new_object, LinearAllocationBuffer* dest_lab,
    size_t dest_offset)
    : ObjectVisitorWithCageBases(serializer->isolate()),
      serializer_(serializer),
      old_object_(old_object),
      new_object_(new_object),
      dest_lab_(dest_lab) {}

void FastSerializer::ObjectSerializer::SerializeObject() {
  // We get the object in its new location if we are relocating objects to
  // synthetic labs.

  // TODO: Do we need to avoid the visitors processing weakness.  Search for
  // "Descriptor arrays have complex element weakness".
  // TODO: WTF UnlinkWeakNextScope unlink_weak_next(isolate()->heap(), object_);
  // Iterate references.  This will call some of the virtuals on the
  // ObjectSerializer.
  VisitObject(serializer_->isolate(), old_object_, this);
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
  Address base_address = old_object_.address();
  for (CompressedMaybeObjectSlot slot = start; slot < end; slot++) {
    size_t in_object_offset = slot.address() - base_address;
    size_t in_lab_offset = new_object_->address() -
                           reinterpret_cast<Address>(dest_lab_->BackingAt(0));
    CompressedMaybeObjectSlot new_slot(new_object_.address() +
                                       in_object_offset);
    serializer_->VisitSlot(dest_lab_, in_lab_offset + in_object_offset, slot,
                           new_slot);
  }
}

void FastSerializer::ObjectSerializer::VisitMapPointer(
    Tagged<HeapObject> host) {
  // Nothing special about the map for us.
  VisitPointers(host, host->map_slot(), host->map_slot());
}

void FastSerializer::ObjectSerializer::VisitInstructionStreamPointer(
    Tagged<Code> host, InstructionStreamSlot slot) {
  Tagged<Object> maybe_instruction_stream = slot.load(code_cage_base());
  if (maybe_instruction_stream.IsSmi()) {
    return;
  }
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

void FastSerializer::MarkTableEntry(LinearAllocationBuffer* table_lab,
                                    Address entry_address, size_t entry_size) {
  table_lab->Expand(entry_address, entry_address + entry_size);
  if (!IsMarked(entry_address)) {
    // We don't push the object onto the queue, that's the responsibility of
    // the caller.
    Mark(entry_address, entry_size);
  }
}

void FastSerializer::ObjectSerializer::VisitExternalPointer(
    Tagged<HeapObject> host, ExternalPointerSlot slot) {
  ExternalPointerTagRange tag_range = slot.tag_range();
  CHECK(!tag_range.IsEmpty());
  CHECK(!IsSharedExternalPointerType(tag_range));
  if (serializer_->is_read_only()) {
    CHECK(IsMaybeReadOnlyExternalPointerType(tag_range));
    ExternalPointerHandle handle = slot.Relaxed_LoadHandle();
    ExternalPointerTable& table =
        serializer_->isolate()->external_pointer_table();
    Address table_base = table.base();
    uint32_t handle_index = table.HandleToIndex(handle);
    Address slot_address =
        table_base + handle_index * sizeof(ExternalPointerTableEntry);
    if ((serializer_->flags_ & Snapshot::kUseIsolateMemory) != 0) {
      LinearAllocationBuffer* table_lab;
      // TODO: Not sure how to get the correct space here, using OLD_SPACE for
      // now.
      table_lab = serializer_->GetOrCreateLabForFixedLocation(
          slot_address, kTrustedPointerTable, OLD_SPACE);
      serializer_->MarkTableEntry(table_lab, slot_address,
                                  sizeof(ExternalPointerTableEntry));
      // No relocation needed, since we are not changing the entry in the
      // external pointer table, but we need to handle the actual pointers in
      // the external pointer table.
    } else {
      // TODO: Allocate a slot in a virtualized ExternalPointerTable.
      // TODO: Emit a relocation for the index into the table. The table is not
      // compressed, but the indices are shifted for security.
    }
  } else {
    // TODO: Handle external pointers outside the read-only snapshot.
    UNREACHABLE();
  }
}

void FastSerializer::ObjectSerializer::VisitProtectedPointer(
    Tagged<TrustedObject> _,
    OffHeapCompressedObjectSlot<V8HeapCompressionSchemeImpl<TrustedCage>>
        slot) {
  // The slot is a compressed offset in the trusted cage.
  Address base_address = old_object_.address();
  size_t in_object_offset = slot.address() - base_address;
  size_t in_lab_offset = new_object_->address() -
                         reinterpret_cast<Address>(dest_lab_->BackingAt(0));
  OffHeapCompressedObjectSlot<V8HeapCompressionSchemeImpl<TrustedCage>>
      new_slot(new_object_.address() + in_object_offset);
  serializer_->VisitSlot(dest_lab_, in_lab_offset + in_object_offset, slot,
                         new_slot);
}

void FastSerializer::ObjectSerializer::VisitProtectedPointer(
    Tagged<TrustedObject>, OffHeapCompressedMaybeObjectSlot<
                               V8HeapCompressionSchemeImpl<TrustedCage>>) {
  UNREACHABLE();
}

void FastSerializer::ObjectSerializer::VisitTrustedPointerTableEntry(
    Tagged<HeapObject> object, IndirectPointerSlot slot) {
  Tagged<Object> value = slot.load(serializer_->isolate());
  VisitIndirectPointerHelper(value, slot);
}

void FastSerializer::ObjectSerializer::VisitIndirectPointerHelper(
    Tagged<Object> value, IndirectPointerSlot slot) {
  // The slot contains an index into the trusted pointer table.
  // Every trusted object has its own index in the trusted pointer table
  // as its first slot.  There can be trusted objects in the read-only
  // space.  It's safe because it is read-only.
  /*
  // Currently not emitting relocation for the index into the table, so
  // we don't need these.
  Address base_address = old_object_.address();
  size_t in_object_offset = slot.address() - base_address;
  size_t in_lab_offset = new_object_->address() -
                         reinterpret_cast<Address>(dest_lab_->BackingAt(0));
  IndirectPointerSlot new_slot(new_object_.address() + in_object_offset,
  slot.tag());
  */

  // Get the index into the TrustedPointerTable.  It's actually just a 32 bit
  // integer.
  IndirectPointerHandle handle = slot.Relaxed_LoadHandle();

  Address entry_address;
  if (slot.tag() == kCodeIndirectPointerTag) {
    CodePointerTable* table =
        serializer_->isolate()->isolate_group()->code_pointer_table();
    uint32_t handle_index = table->HandleToIndex(handle);
    Address table_base = table->base_address();
    entry_address =
        table_base + handle_index * sizeof(TrustedPointerTableEntry);
  } else {
    // Eg a SFI in the read-only space.
    TrustedPointerTable& table =
        serializer_->isolate()->trusted_pointer_table();
    uint32_t handle_index = table.HandleToIndex(handle);
    Address table_base = table.base_address();
    entry_address =
        table_base + handle_index * sizeof(TrustedPointerTableEntry);
  }
  LinearAllocationBuffer* table_lab;
  // Offset within the table lab.
  uintptr_t table_offset;
  if ((serializer_->flags_ & Snapshot::kUseIsolateMemory) != 0) {
    // TODO: Not sure how to get the correct space here, using OLD_SPACE for
    // now.
    table_lab = serializer_->GetOrCreateLabForFixedLocation(
        entry_address, kTrustedPointerTable, OLD_SPACE);
    // Mark the slot in use in its own lab.
    serializer_->MarkTableEntry(table_lab, entry_address,
                                sizeof(TrustedPointerTableEntry));
    table_offset = entry_address - table_lab->start();
    // No relocation is needed in this case for the table index slot in the
    // pointed-from object, since we are not changing it.
  } else {
    // TODO: Allocate a slot in a virtualized TrustedPointerTable.
    // TODO: Emit a relocation for the index into the table. The table is not
    // compressed, but the indices are shifted for security and in the case of
    // the code pointer table there's also a tag in the low bit that we need to
    // account for in the relocation information.
    UNREACHABLE();
  }
  // Deal with the indirectly referenced object that is in the table.
  if (!value.IsSmi()) {
    LabAndOffset location = serializer_->FoundObject(Cast<HeapObject>(value));
    constexpr bool table_slot_uncompressed = false;
    serializer_->snapshot()->AddRelocation(table_lab->index(),
                                           location.lab->index(), table_offset,
                                           table_slot_uncompressed);
  }
}

void FastSerializer::ObjectSerializer::VisitIndirectPointer(
    Tagged<HeapObject> host, IndirectPointerSlot slot,
    IndirectPointerMode mode) {
  Tagged<Object> value =
      slot.Relaxed_Load_AllowUnpublished(serializer_->isolate());
  VisitIndirectPointerHelper(value, slot);
}

void FastSerializer::ObjectSerializer::VisitJSDispatchTableEntry(
    Tagged<HeapObject>, JSDispatchHandle handle) {
  if (serializer_->is_read_only()) {
    // There is a read-only root which is the singleton many_closures_cell
    // which contains an unused index into the JSDispatchTable.
    CHECK_EQ(handle, kNullJSDispatchHandle);
  } else {
    // TODO: Handle JSDispatchTable indices.
    UNREACHABLE();
  }
}

const char* SpaceToName(AddressSpace space) {
  switch (space) {
    case kUncompressed:
      return "Uncompressed";
    case kMainCage:
      return "MainCage";
    case kTrustedCage:
      return "TrustedCage";
    case kCodeSpace:
      return "CodeSpace";
    case kRoots:
      return "Roots";
    case kExternalPointerTable:
      return "ExternalPointerTable";
    case kTrustedPointerTable:
      return "TrustedPointerTable";
    case kJSDispatchTable:
      return "JSDispatchTable";
    default:
      return "Unknown space";
  }
}

// For debugging only.
void FastSerializer::Dump() {
  for (LinearAllocationBuffer* lab : all_labs_) {
    Address lab_base = lab->start();
    uint64_t* liveness_map = lab_liveness_map_[lab_base];
    const char* ro_tag = "";
    AddressSpace space = lab->address_space();
    if (space == kMainCage && ReadOnlyHeap::Contains(lab_base)) {
      ro_tag = "Read-only ";
    }
    printf("%sLab at %p in %s:\n", ro_tag, (void*)lab_base,
           SpaceToName(lab->address_space()));
    constexpr int kWordSize = sizeof(uint32_t);  // Words on a compressed heap.
    char line_buffer[1000];
    char old_line_buffer[1000];
    memset(old_line_buffer, 0, sizeof(old_line_buffer));

    int line_pos = 0;
    bool dots_printed = false;
    constexpr size_t kLineSize = kWordSize * 64;
    for (int i = 0; i <= kRegularPageSize; i += kWordSize) {
      int word_index = i / kWordSize;  // Compressed words are 32 bits.
      if (i % kLineSize == 0 && i != 0) {
        line_buffer[line_pos] = 0;
        if (strcmp(line_buffer, old_line_buffer) != 0 ||
            i == kRegularPageSize) {
          printf("0x%016lx %s - %3.2fk-%3.2fk\n", lab_base + i - kLineSize,
                 line_buffer, (i - kLineSize) / 1024.0, i / 1024.0);
          dots_printed = false;
          memcpy(old_line_buffer, line_buffer, sizeof(old_line_buffer));
        } else {
          if (!dots_printed) {
            printf("...                %s\n", line_buffer);
            dots_printed = true;
          }
        }
        line_pos = 0;
        if (i == kRegularPageSize) break;
      }
      int array_index = word_index / 64;  // 64 bits in a mark word.
      uint64_t word = liveness_map[array_index];
      int in_mark_word_index = word_index % 64;
      if ((word & (uint64_t{1} << in_mark_word_index)) != 0) {
        line_pos += sprintf(line_buffer + line_pos, "*");
      } else {
        line_pos += sprintf(line_buffer + line_pos, ".");
      }
    }
  }
}

}  // namespace internal
}  // namespace v8
