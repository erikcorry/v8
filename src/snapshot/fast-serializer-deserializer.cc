// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer-deserializer.h"

#include "src/base/build_config.h"
#include "src/objects/embedder-data-array-inl.h"
#include "src/objects/objects-inl.h"

namespace v8 {
namespace internal {

LinearAllocationBuffer::LinearAllocationBuffer(int index, AllocationSpace space, Address lowest, Address highest) :
    lab_index_(index),
    space_(space),
    start_(RoundDown(lowest, kRegularPageSize)),
    lowest_(lowest),
    highest_(highest) {}

FastSnapshot::FastSnapshot() :
    zone_(&allocator_, "FastSnapshot"),
    labs_(&zone_),
    relocations_(&zone_) {}

}  // namespace internal
}  // namespace v8
