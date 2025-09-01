// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer-deserializer.h"

#include "src/snapshot/snapshot.h"
#include "src/snapshot/fast-serializer.h"
#include "include/v8-value-serializer.h"
#include "src/base/build_config.h"
#include "src/objects/embedder-data-array-inl.h"
#include "src/objects/objects-inl.h"

namespace v8 {
namespace internal {

LinearAllocationBuffer::LinearAllocationBuffer(Zone* zone, int index,
                                               AllocationSpace space,
                                               Address lowest, Address highest)
    : lab_index_(index),
      points_to_(zone),
      space_(space),
      start_(RoundDown(lowest, kRegularPageSize)),
      lowest_(lowest),
      highest_(highest) {}

void LinearAllocationBuffer::SetPointsTo(int lab) {
  size_t index = lab >> 6;
  while (points_to_.size() <= index) points_to_.push_back(0);
  points_to_[index] |= uint64_t{1} << (lab & 0x3f);
}

bool LinearAllocationBuffer::PointsTo(int lab) const {
  size_t index = lab >> 6;
  DCHECK(index < points_to_.size());
  return (points_to_[index] & (uint64_t{1} << (lab & 0x3f))) != 0;
}

FastSnapshot::FastSnapshot()
    : zone_(&allocator_, "FastSnapshot"),
      labs_(&zone_),
      relocations_(&zone_),
      roots_instructions_(&zone_) {}

void foo() { FastSnapshot snapshot; }


FastSnapshotCreatorImpl::FastSnapshotCreatorImpl(Isolate* isolate) : isolate_(isolate) {}

FastSnapshotCreatorImpl::~FastSnapshotCreatorImpl() {

}

void FastSnapshotCreatorImpl::TakeSnapshot() {
  FastSerializer serializer(this->isolate_, Snapshot::kDefaultSerializerFlags);
  this->snapshot_ = serializer.Run();
}

void FastSnapshotCreatorImpl::ApplySnapshot(Isolate* isolate) {

}

}  // namespace internal
}  // namespace v8
