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
      snapshot_(new FastSnapshot()) {
  // Nothing points at the roots, so the location is not important.  We use the
  // zero location for this lab.
  Address location = 0;
  int lab_index = 0;
  roots_lab_ = zone_.New<LinearAllocationBuffer>(
      &zone_, lab_index++, kIgnoreAllocationSpace, AddressSpace::kRoots,
      location, location);
  for (int space = 0; space < kNumberOfAddressSpaces; space++) {
    address_space_fullnesses_[space] = 0;
  }
  for (int space = FIRST_SPACE; space <= LAST_SPACE; space++) {
    lab_per_space_[space] = nullptr;
  }
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
  MemoryChunkMetadata* metadata = MemoryChunkMetadata::FromHeapObject(object);
  return metadata->owner()->identity();
}

bool FastSerializer::IsMarked(Tagged<HeapObject> object) {
  Address lab_start = RoundDown(object->address(), kRegularPageSize);
  uint64_t*& bitmap = lab_liveness_map_[lab_start];
  if (bitmap == nullptr) {
    // We don't yet have a liveness map for this lab, so we allocate one.
    // Each 32 bit word is a bit in the map so we divide the size with 32.
    bitmap = reinterpret_cast<uint64_t*>(
        zone_.Allocate<uint64_t>(kRegularPageSize / 32));
  }
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
  // We already called IsMarked so the bitmap must exist.
  DCHECK(bitmap != nullptr);

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
    constexpr bool compressed = false;
    VisitSlot(roots_lab_, offset, *current, compressed);
  }
}

void FastSerializer::VisitSlot(
    LinearAllocationBuffer* slot_lab,  // Lab containing slot.
    size_t slot_offset,                // Position of slot in slot_lab.
    Tagged<Object> maybe_smi,          // Object pointed to by slot.
    bool compressed_slot) {
  if (maybe_smi.IsSmi()) return;
  Tagged<HeapObject> slot_contents = Cast<HeapObject>(maybe_smi);
  Address start = slot_contents.address();
  size_t size = slot_contents->Size();
  if ((flags_ & Snapshot::kUseIsolateMemory) != 0) {
    // The labs just record the real location of the objects.
    LinearAllocationBuffer* lab = GetOrCreateLabForFixedLocation(slot_contents);
    if (lab->index() != slot_lab->index()) {
      snapshot_->AddRelocation(slot_lab->index(), lab->index(), slot_offset,
                               compressed_slot);
    }
    lab->Expand(start, start + size);
    if (!IsMarked(slot_contents)) {
      Mark(slot_contents, size);
      queue_.push_back(slot_contents);
    }
  } else {
    if (!IsMarked(slot_contents)) {
      Mark(slot_contents, size);
      queue_.push_back(slot_contents);
      // The objects are reallocated in tightly packed labs that we create
      // in the serializer for use by the deserializer.
      LinearAllocationBuffer* lab = GetOrCreatePackedLab(slot_contents, size);
      size_t offset = lab->highest();
      lab->Expand(offset, offset + size);
      // We may overwrite parts of the object later, but for now we copy
      // everything into the allocation.
      memcpy(lab->BackingAt(offset), reinterpret_cast<void*>(start), size);
      if (compressed_slot) {
        CHECK(offset - lab->start() < 0x10000000ULL);
        uint32_t offset_in_lab = static_cast<uint32_t>(offset - lab->start());
        *reinterpret_cast<uint32_t*>(slot_lab->BackingAt(slot_offset)) =
            offset_in_lab;
      } else {
        *reinterpret_cast<Address*>(slot_lab->BackingAt(slot_offset)) = offset;
      }
      snapshot_->AddRelocation(slot_lab->index(), lab->index(), slot_offset,
                               compressed_slot);
      new_locations_[start] = {lab, offset};
    } else {
      LabAndOffset lao = new_locations_[start];
      if (compressed_slot) {
        CHECK(lao.offset < 0x10000000ULL);
        uint32_t offset_in_lab = static_cast<uint32_t>(lao.offset);
        *reinterpret_cast<uint32_t*>(slot_lab->BackingAt(slot_offset)) =
            offset_in_lab;
      } else {
        *reinterpret_cast<Address*>(slot_lab->BackingAt(slot_offset)) =
            lao.offset;
      }
      snapshot_->AddRelocation(slot_lab->index(), lao.lab->index(), lao.offset,
                               compressed_slot);
    }
  }
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
    lab = zone_.New<LinearAllocationBuffer>(&zone_, all_labs_.size(), space,
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
          &zone_, all_labs_.size(), allocation_space, address_space, fullness);
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
      LabAndOffset lao = new_locations_[object.address()];
      dest_lab = lao.lab;
      dest_offset = lao.offset;
    } else {
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
  Tagged<Map> map = object_->map(serializer_->cage_base());
  // TODO: Do we need to avoid the visitors processing weakness.  Search for
  // "Descriptor arrays have complex element weakness".
  // Visit the map field.  The map is always in the main cage, so the base is
  // given.
  constexpr bool compressed = true;
  serializer_->VisitSlot(dest_lab_, dest_offset_, map, compressed);
  // TODO: WTF UnlinkWeakNextScope unlink_weak_next(isolate()->heap(), object_);
  // Iterate references.
  VisitObjectBody(serializer_->isolate(), map, object_, this);
  // Nothing to do for the data payload.
}

>>>>>>> 6d043a32af1 (wip)
}  // namespace internal
}  // namespace v8
