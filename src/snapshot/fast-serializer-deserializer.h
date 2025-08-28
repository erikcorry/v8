// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_
#define V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_

#include "src/common/globals.h"
#include "src/objects/visitors.h"
#include "src/snapshot/references.h"
#include "src/zone/accounting-allocator.h"
#include "src/zone/zone.h"

namespace v8 {
namespace internal {

class Isolate;

class LinearAllocationBuffer {
 public:
  LinearAllocationBuffer(int index, AllocationSpace space, Address lowest, Address highest);

  int index() const { return lab_index_; }
  AllocationSpace space() const { return space_; }
  Address start() const { return start_; }
  Address lowest() const { return lowest_; }
  Address highest() const { return highest_; }

  void expand(Address from, Address to) {
    DCHECK(from >= start_);
    DCHECK(to <= start_ + kRegularPageSize);
    if (lowest_ > from) lowest_ = from;
    if (highest_ < to) highest_ = to;
  }

 private:
  int lab_index_;          // Unique in a given snapshot.
  AllocationSpace space_;  // Enum of the space type.
  Address start_;          // Location of start of 128k page.
  Address lowest_;         // Address of lowest object in lab.
  Address highest_;        // Address of end of highest object in lab.
};


class LabMap
    : public base::TemplateHashMapImpl<Address, LinearAllocationBuffer*,
                                       base::KeyEqualityMatcher<Address>,
                                       ZoneAllocationPolicy> {
 public:
  LabMap(Zone* zone) : allocation_policy_(zone) {}

  inline void Set(Address key, LinearAllocationBuffer* value) {
    LookupOrInsert(key, Hash(key))->value = value;
  }

  /*
  inline LinearAllocationBuffer* Get(Address key) const {
    using Entry = base::TemplateHashMapEntry<uintptr_t, LinearAllocationBuffer*>;
    Entry* entry = Lookup(key, Hash(key));
    if (entry == nullptr) return nullptr;
    return entry->value;
  }
  */

 private:
  static uint32_t Hash(Address key) { return static_cast<uint32_t>(key ^ (key >> 32)); }

  ZoneAllocationPolicy allocation_policy_;
};


// The FastSnapshot is an in-memory representation of a snapshot.  The
// serialized snapshot format is not yet written.  FastSnapshot needs
// GC to be paused while it deserializes.
class FastSnapshot {
 public:
  FastSnapshot();

 private:
  class AddressMatcher : public base::KeyEqualityMatcher<Address> {
   public:
    bool operator()(uint32_t hash1, uint32_t hash2, const Address& key1,
                    const Address& key2) const {
      return key1 == key2;
    }
  };

  AccountingAllocator allocator_;  // For the zone.
  Zone zone_; 
  LabMap labs_; 
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_
