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
// one 256k page (unless they are in a large object space). Often they
// will hopefully be much smaller.
class LinearAllocationBuffer {
 public:
  LinearAllocationBuffer(Zone* zone, size_t index, AllocationSpace space,
                         bool is_compressed, Address lowest, Address highest);

  size_t index() const { return lab_index_; }
  AllocationSpace space() const { return space_; }
  bool is_compressed() const { return is_compressed_; }
  Address start() const { return start_; }
  Address lowest() const { return lowest_; }
  Address highest() const { return highest_; }

  void Expand(Address from, Address to) {
    DCHECK(from >= start_);
    if (lowest_ > from) lowest_ = from;
    if (highest_ < to) highest_ = to;
  }

  // Set and get the set of labs this lab points at.
  void SetPointsTo(int other_index);
  bool PointsTo(int other_index) const;

 private:
  size_t lab_index_;                // Unique in a given snapshot.
  ZoneVector<uint64_t> points_to_;  // Bitmap of other labs this one points at.
  AllocationSpace space_;           // Enum of the space type.
  bool is_compressed_;              // If false, slots are pointer-sized.
  Address start_;                   // Location of start of 256k page.
  Address lowest_;                  // Address of lowest object in lab.
  Address highest_;                 // Address of end of highest object in lab.
};

// Slots in objects that might need relocating after a deserialization.
class Relocation {
 public:
  Relocation(size_t source_lab, size_t destination_lab, size_t offset_in_source)
      : source_lab_(source_lab),
        destination_lab_(destination_lab),
        offset_(offset_in_source) {}

  size_t source_lab() const { return source_lab_; }
  size_t destination_lab() const { return destination_lab_; }
  size_t offset() const { return offset_; }

 private:
  size_t source_lab_;       // The lab containing the slot.
  size_t destination_lab_;  // The lab the slot is pointing to.
  size_t offset_;           // Location of the slot within the source, in bytes.
};

// The FastSnapshot is an in-memory representation of a snapshot.  The
// serialized snapshot format is not yet written.  FastSnapshot needs
// GC to be paused while it deserializes.
class FastSnapshot {
 public:
  FastSnapshot();

 private:
  LinearAllocationBuffer* FindOrCreateLab(Address for_address,
                                          AllocationSpace space,
                                          bool is_compressed);

  void AddRelocation(size_t source_lab, size_t destination_lab,
                     size_t slot_offset);

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
  // These are the special fixups that have to be done at the end because
  // they don't fit in our normal way of copying and relocating.
  // It's a vector of uint32_t's which probably will be based on some sort
  // of bytecode system.
  SmallZoneVector<uint32_t, 10> remaining_fixups_;
  // The data in the pseudo-lab that backs the roots.
  SmallZoneVector<Address, 10> root_lab_data_;

  friend class FastSerializer;
};

class FastSnapshotCreatorImpl final {
 public:
  FastSnapshotCreatorImpl(Isolate* isolate);

  ~FastSnapshotCreatorImpl();

  void TakeSnapshot();
  void ApplySnapshot(Isolate* isolate);

 private:
  Isolate* const isolate_;
  std::unique_ptr<FastSnapshot> snapshot_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_DESERIALIZER_H_
