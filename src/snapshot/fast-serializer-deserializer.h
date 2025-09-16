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

// Address spaces are mappings from integers to objects and slots within
// objects.
enum AddressSpace {
  kUncompressed,  // The uncaged 64 bit address space.
  kMainCage,      // The first 4Gbytes of the sandbox.
                  // This is a 32 bit space.
  kTrustedCage,   // Trusted objects in their own compression cage.
                  // Maybe a mixture of 32 bit and 64 bit :-/.
  kCodeSpace,     // Executable machine code.
               // This may require custom relocation due to unaligned relative
               // pointers on x64.
  kSharedSpace,  // The read-only shared space for immortal objects.
  kRoots,        // The roots and global handles in the order they are visited.
                 // This is a 64 bit space.
  kExternalPointerTable,  // Entries in the external pointer table.  64 bit.
  // Other tables ha.ve their own address spaces.
  // Oilpan perhaps?
  kNumberOfAddressSpaces
};

// LAB: An area that contains objects that are part of the snapshot.
// A lab is always smaller than a 256k page, and always contained in
// one 256k page (unless they are in a large object space). Often they
// will hopefully be much smaller.
class LinearAllocationBuffer {
 public:
  // Constructor that makes a lab backed by the old heap, which means it's
  // in a fixed place and has no backing store of its own.
  LinearAllocationBuffer(Zone* zone, size_t index, AllocationSpace space,
                         AddressSpace address_space, Address lowest,
                         Address highest);
  // Constructor that makes an empty lab with its own backing.
  LinearAllocationBuffer(Zone* zone, size_t index, AllocationSpace space,
                         AddressSpace address_space, Address rounded_address);

  size_t index() const { return lab_index_; }
  AllocationSpace space() const { return space_; }
  AddressSpace address_space() const { return address_space_; }
  bool is_compressed() const;
  Address start() const { return start_; }
  Address lowest() const { return lowest_; }
  Address highest() const { return highest_; }

  // Set highest/lowest and extend the backing if there is one.
  void Expand(Address from, Address to);

  // A short-lived pointer into the backing.
  uint8_t* BackingAt(size_t offset);

  // Set and get the set of labs this lab points at.
  void SetPointsTo(int other_index);
  bool PointsTo(int other_index) const;

 private:
  size_t lab_index_;                // Unique in a given snapshot.
  ZoneVector<uint64_t> points_to_;  // Bitmap of other labs this one points at.
  AllocationSpace space_;           // Enum of which heap space we are in.
  AddressSpace address_space_;      // Enum of which cage or table we are in.
  bool own_backing_;  // Do we have a backing array, or are we just using the
                      // old heap.
  ZoneVector<uint8_t> backing_;  // The actual bytes of the lab if it is not
                                 // backed by the old heap.
  Address start_;                // Location of start of 256k page.
  Address lowest_;               // Address of lowest object in lab.  Mutable.
  Address highest_;  // Address of end of highest object in lab.  Mutable.
};

// Slots in objects that might need relocating after a deserialization.
// Whether the slot is 32 bit or 64 bit can be determined by the source lab.
// TODO: Code relocs in x64 code have two complications: They are relative, and
// they are unaligned.  Within one lab nothing needs to be done, but we can't
// support inter-lab code relocs with this struct.
class Relocation {
 public:
  Relocation(size_t source_lab, size_t destination_lab, size_t offset_in_source,
             bool compressed)
      : source_lab_(source_lab),
        destination_lab_(destination_lab),
        offset_(offset_in_source),
        compressed_(compressed) {}

  size_t source_lab() const { return source_lab_; }
  size_t destination_lab() const { return destination_lab_; }
  size_t offset() const { return offset_; }
  bool compressed() const { return compressed_; }

 private:
  size_t source_lab_;       // The lab containing the slot.
  size_t destination_lab_;  // The lab the slot is pointing to.
  size_t offset_;           // Location of the slot within the source, in bytes.
  bool compressed_;         // 32 bit or 64 bit field.
};

// The FastSnapshot is an in-memory representation of a snapshot.  The
// serialized snapshot format is not yet written.  FastSnapshot needs
// GC to be paused while it deserializes.
class FastSnapshot {
 public:
  FastSnapshot();

 private:
  void AddRelocation(size_t source_lab, size_t destination_lab,
                     size_t slot_offset, bool compressed);

  class AddressMatcher : public base::KeyEqualityMatcher<Address> {
   public:
    bool operator()(uint32_t hash1, uint32_t hash2, const Address& key1,
                    const Address& key2) const {
      return key1 == key2;
    }
  };

  AccountingAllocator allocator_;  // For the zone.
  Zone zone_;
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
