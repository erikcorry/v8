// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_READ_ONLY_SERIALIZER_H_
#define V8_SNAPSHOT_READ_ONLY_SERIALIZER_H_

#include "src/snapshot/roots-serializer.h"
#include "src/zone/zone-containers.h"
#include "src/zone/zone.h"

namespace v8 {
namespace internal {

// When building the read-only snapshot we use this to make
// a mirror of the code pointer table entries that are used
// by the snapshot.
class VirtualCodePointerTable {
 public:
  VirtualCodePointerTable(Zone* zone) : used_slots_(zone), entries_(zone) {}

  bool is_free(size_t slot) const {
    if (slot >= used_slots_.size()) return true;
    return used_slots_[slot] == 0;
  }

  void mark_in_use(size_t slot) {
    DCHECK(is_free(slot));
    if (last_in_use_ < slot) last_in_use_ = slot;
    if (used_slots_.size() <= slot) used_slots_.resize(slot + 1, 0);
    used_slots_[slot] = 1;
  }

  void set_entry(size_t slot, Address code_object, Address tagged_entry_point) {
    if (entries_.size() <= slot) entries_.resize(slot + 1);
    entries_[slot] = VirtualCodePointerEntry(code_object, tagged_entry_point);
  }

 private:
  struct VirtualCodePointerEntry {
    VirtualCodePointerEntry() : code_object(0), tagged_entry_point(0) {}
    VirtualCodePointerEntry(Address c, Address e)
        : code_object(c), tagged_entry_point(e) {}
    Address code_object;
    Address tagged_entry_point;
  };

  ZoneVector<uint8_t> used_slots_;
  ZoneVector<VirtualCodePointerEntry> entries_;
  size_t last_in_use_ = 0;
};

// TODO(jgruber): Now that this does a memcpy-style serialization, there is no
// longer a fundamental reason to inherit from RootsSerializer. It's still
// convenient though because callers expect parts of the Serializer interface
// (e.g.: rehashability, serialization statistics, blob creation).
// Consider removing this inheritance.
class V8_EXPORT_PRIVATE ReadOnlySerializer : public RootsSerializer {
 public:
  ReadOnlySerializer(Isolate* isolate, Snapshot::SerializerFlags flags);
  ~ReadOnlySerializer() override;

  // Serializes the entire ReadOnlySpace as well as the ReadOnlyRoots table.
  void Serialize();

 private:
  void SerializeObjectImpl(Handle<HeapObject> o, SlotType slot_type) override {
    UNREACHABLE();
  }

  ReadOnlySerializer(const ReadOnlySerializer&) = delete;
  ReadOnlySerializer& operator=(const ReadOnlySerializer&) = delete;

  Zone zone_;
  VirtualCodePointerTable code_pointer_table_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_READ_ONLY_SERIALIZER_H_
