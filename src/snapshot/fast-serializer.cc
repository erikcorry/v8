// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer.h"

namespace v8 {
namespace internal {

FastSerializer::~FastSerializer() {}

bool FastSerializer::IsMarked(Tagged<HeapObject> object) {
  Address lab_start = RoundDown(object->address(), kRegularPageSize);
  uint64_t*& bitmap = lab_liveness_map_[lab_start];
  if (bitmap == nullptr) {
    // We don't yet have a liveness map for this lab, so we allocate one.
    // Each 32 bit word is a bit in the map so we divide the size with 32.
    bitmap = reinterpret_cast<uint64_t*>(zone_.Allocate<uint64_t>(kRegularPageSize / 32));
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

}  // namespace internal
}  // namespace v8
