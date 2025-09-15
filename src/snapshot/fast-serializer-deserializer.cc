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

// Constructor that makes a lab backed by the old heap, which means it's in a
// fixed place and has no backing store of its own.
LinearAllocationBuffer::LinearAllocationBuffer(Zone* zone, size_t index,
                                               AllocationSpace space,
                                               AddressSpace address_space,
                                               Address lowest, Address highest)
    : lab_index_(index),
      points_to_(zone),
      space_(space),
      address_space_(address_space),
      own_backing_(false),
      backing_(zone),
      start_(RoundDown(lowest, kRegularPageSize)),
      lowest_(lowest),
      highest_(highest) {}

// Constructor that makes an empty lab with its own backing.
LinearAllocationBuffer::LinearAllocationBuffer(Zone* zone, size_t index,
                                               AllocationSpace space,
                                               AddressSpace address_space,
                                               Address rounded_address)
    : lab_index_(index),
      points_to_(zone),
      space_(space),
      address_space_(address_space),
      own_backing_(true),
      backing_(zone),
      start_(rounded_address),
      lowest_(rounded_address),
      highest_(rounded_address) {}

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
      remaining_fixups_(&zone_),
      root_lab_data_(&zone_) {}

LinearAllocationBuffer* FastSnapshot::FindOrCreateFixedLocationLab(
    Address start, AllocationSpace space_enum, AddressSpace address_space) {
  Address rounded_address = RoundDown(start, kRegularPageSize);
  LinearAllocationBuffer*& lab = labs_[rounded_address];
  if (lab == nullptr) {
    // Updates collection because it's a reference.
    lab = zone_.New<LinearAllocationBuffer>(&zone_, labs_.size(), space_enum,
                                            address_space, start, start);
  }
  return lab;
}

void FastSnapshot::AddRelocation(size_t source_lab, size_t dest_lab,
                                 size_t slot_offset) {
  if (source_lab != dest_lab) {
    relocations_.push_back(Relocation(source_lab, dest_lab, slot_offset));
  }
}

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
