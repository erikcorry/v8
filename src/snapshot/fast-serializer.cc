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
      lab_liveness_maps_{
          // Note: If the compiler complains that there are zero args when
          // there should be 1 that probably just means there are not enough
          // lines here.
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_},
          ZoneAbslFlatHashMap<Address, uint64_t*>{&zone_}
      },
      cage_base_(isolate),
      external_reference_encoder_(isolate),
      flags_(flags),
      snapshot_(new FastSnapshot()) {}

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
  AddressSpace address_space = GetAddressSpace(object);
  Address lab_start = RoundDown(object->address(), kRegularPageSize);
  uint64_t*& bitmap = lab_liveness_maps_[address_space][lab_start];
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
  AddressSpace address_space = GetAddressSpace(object);
  auto it = lab_liveness_maps_[address_space].find(lab_start);
  uint64_t* bitmap = (it == lab_liveness_maps_[address_space].end()) ? nullptr : it->second;
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
  // Nothing points at the roots, so the location is not important.  We use the
  // zero location for this lab.
  LinearAllocationBuffer* lab = snapshot_->FindOrCreateFixedLocationLab(
      0, kIgnoreAllocationSpace, AddressSpace::kRoots);
  for (FullObjectSlot current = start; current < end; ++current) {
    size_t offset = lab->highest();
    lab->Expand(offset, offset + sizeof(Address));
    snapshot_->root_lab_data_.push_back((*current).ptr());
    VisitUncompressedSlot(0, offset, *current);
  }
}

void FastSerializer::VisitUncompressedSlot(
    size_t source_lab,   // Lab that contains slot.
    size_t slot_offset,  // Position of slot in source lab.
    Tagged<Object> maybe_smi) {  // Object pointed to by slot.
  if (maybe_smi.IsSmi()) return;
  Tagged<HeapObject> slot_contents = Cast<HeapObject>(maybe_smi);
  LinearAllocationBuffer* lab = GetOrCreateLab(slot_contents);
  if (lab->index() != source_lab) {
    snapshot_->AddRelocation(source_lab, lab->index(), slot_offset);
  }
  if (!IsMarked(slot_contents)) {
    size_t size = slot_contents->Size();
    Mark(slot_contents, size);
    queue_.push_back(slot_contents);
  }
}

LinearAllocationBuffer* FastSerializer::GetOrCreateLabForFixedLocation(
    Tagged<HeapObject> object) {
  DCHECK((flags_ & Snapshot::kUseIsolateMemory) != 0);
  DCHECK((flags_ & Snapshot::kAllInOneSpace) == 0);
  AddressSpace heap_space = GetAddressSpace(object);
  Address start = RoundDown(object.address(), kRegularPageSize);
  Tagged<Map> map = object->map(cage_base());
  int size = object->SizeFromMap(map);

  constexpr bool is_compressed = true;
  AllocationSpace space = GetAllocationSpace(object);
  LinearAllocationBuffer* lab =
      snapshot_->FindOrCreateFixedLocationLab(start, space, heap_space);
  return lab;
}

FastSnapshot* SerializeIsolate() {
  // TODO.
  return nullptr;
}

void FastSerializer::ProcessQueue() {
  while (!queue_empty()) {
    Tagged<HeapObject> object = queue_.back();
    queue_.pop_back();
    ObjectSerializer serializer(this, object);
    serializer.SerializeObject();
  }
}

void FastSerializer::ObjectSerializer::SerializeObject() {
  Tagged<Map> map = object_->map(serializer_->cage_base());
  int size = object_->SizeFromMap(map);
  // TODO: Do we need to avoid the visitors processing weakness.  Search for
  // "Descriptor arrays have complex element weakness".
  // Visit the map field.  The map is always in the main cage, so the base is given.
  LinearAllocationBuffer* source_lab = serializer_->GetOrCreateLab(object_);
  size_t offset = object_->address() - source_lab->start();
  source_lab->Expand(offset, offset + size);
  LinearAllocationBuffer* map_lab = serializer_->GetOrCreateLab(map);
  serializer_->VisitCompressedSlot(source_lab->index(), map_lab->index(), offset, map);
  // TODO: WTF UnlinkWeakNextScope unlink_weak_next(isolate()->heap(), object_);
  // Iterate references.
  VisitObjectBody(serializer_->isolate(), map, object_, this);
  // Nothing to do for the data payload.
}

>>>>>>> 6d043a32af1 (wip)
}  // namespace internal
}  // namespace v8
