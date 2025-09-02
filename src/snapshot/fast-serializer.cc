// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer.h"

#include "src/common/ptr-compr-inl.h"
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
      cage_base_(isolate),
      external_reference_encoder_(isolate),
      flags_(flags),
      snapshot_(new FastSnapshot()) {}

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
  // Nothing points at the roots, so the location is not important.  We use the
  // zero location for this lab.
  constexpr bool is_compressed = false;
  LinearAllocationBuffer* lab = snapshot_->FindOrCreateLab(
      0, AllocationSpace::ROOT_PSEUDO_SPACE, is_compressed);
  for (FullObjectSlot current = start; current < end; ++current) {
    size_t offset = lab->highest();
    lab->Expand(offset, offset + sizeof(Address));
    snapshot_->root_lab_data_.push_back((*current).ptr());
    VisitUncompressedSlot(0, *current);
  }
}

void FastSerializer::VisitUncompressedSlot(size_t source_lab,
                                           Tagged<Object> slot_contents) {
  if (slot_contents.IsSmi()) return;
  Address address = Cast<HeapObject>(slot_contents).address();
  GetOrCreateLab(address);
}

LinearAllocationBuffer* FastSerializer::GetOrCreateLab(Address address) {
  // Address start = RoundDown(address, kRegularPageSize);
  return nullptr;
}

>>>>>>> 492868d4aeb (Create pseudo-lab for roots)
}  // namespace internal
}  // namespace v8
