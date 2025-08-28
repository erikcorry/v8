// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_
#define V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_

#include "src/common/globals.h"
#include "src/objects/visitors.h"
#include "src/snapshot/references.h"
#include "src/zone/zone-containers.h"
#include "src/zone/zone.h"

namespace v8 {
namespace internal {

class Isolate;

// LAB: An area that contains objects that are part of the snapshot.
// A lab is always smaller than a 256k page, and always contained in
// one 256k page. Often they will be much smaller.
class LinearAllocationBuffer {
 public:
  LinearAllocationBuffer(Zone* zone, int index, AllocationSpace space,
                         Address lowest, Address highest);

  int index() const { return lab_index_; }
  AllocationSpace space() const { return space_; }
  Address start() const { return start_; }
  Address lowest() const { return lowest_; }
  Address highest() const { return highest_; }

  void Expand(Address from, Address to) {
    DCHECK(from >= start_);
    DCHECK(to <= start_ + kRegularPageSize);
    if (lowest_ > from) lowest_ = from;
    if (highest_ < to) highest_ = to;
  }

  void SetPointsTo(int other);
  bool PointsTo(int other) const;

 private:
  int lab_index_;                   // Unique in a given snapshot.
  ZoneVector<uint64_t> points_to_;  // Bitmap of other labs this one points at.
  AllocationSpace space_;           // Enum of the space type.
  Address start_;                   // Location of start of 256k page.
  Address lowest_;                  // Address of lowest object in lab.
  Address highest_;                 // Address of end of highest object in lab.
};

// Slots in objects that might need relocating after a deserialization.
class Relocation {
 public:
  Relocation(int source_lab, int destination_lab, int offset_in_source)
      : source_lab_(source_lab),
        destination_lab_(destination_lab),
        offset_(offset_in_source) {}

  int source_lab() const { return source_lab_; }
  int destination_lab() const { return destination_lab_; }
  int offset() const { return offset_; }

 private:
  int source_lab_;       // The lab containing the slot.
  int destination_lab_;  // The lab the slot is pointing to.
  int offset_;           // Location of the slot within the source, in bytes.
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
  ZoneAbslFlatHashMap<Address, LinearAllocationBuffer*> labs_;
  SmallZoneVector<Relocation, 10> relocations_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_
