// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_FAST_SERIALIZER_H_
#define V8_SNAPSHOT_FAST_SERIALIZER_H_

#include "src/codegen/external-reference-encoder.h"
#include "src/common/assert-scope.h"
#include "src/execution/isolate.h"
#include "src/handles/global-handles.h"
#include "src/logging/log.h"
#include "src/objects/abstract-code.h"
#include "src/objects/bytecode-array.h"
#include "src/objects/instruction-stream.h"
#include "src/objects/objects.h"
#include "src/objects/visitors.h"
#include "src/snapshot/fast-serializer-deserializer.h"
#include "src/snapshot/snapshot.h"
#include "src/utils/identity-map.h"

namespace v8 {
namespace internal {

// The 'fast' refers to the speed of deserialization.  The serializer
// itself is not particularly fast.
class FastSerializer : public RootVisitor {
 public:
  FastSerializer(Isolate* isolate, Snapshot::SerializerFlags flags);
  ~FastSerializer();
  FastSerializer(const FastSerializer&) = delete;
  FastSerializer& operator=(const FastSerializer&) = delete;

  Isolate* isolate() const { return isolate_; }

  // The pointer compression cage base value used for decompression of all
  // tagged values except references to InstructionStream objects.
  PtrComprCageBase cage_base() const {
#if V8_COMPRESS_POINTERS
    return cage_base_;
#else
    return PtrComprCageBase{};
#endif  // V8_COMPRESS_POINTERS
  }

  void Serialize();
  FastSnapshot* SerializeContext(Handle<Context> context);

  std::unique_ptr<FastSnapshot> Run();

  // Serialize the transitively reachable objects from the queue,
  // until it is empty.
  void FinalizeSerialization();

  FastSnapshot* snapshot() { return snapshot_; }

  bool can_be_rehashed() const { return false; }

  int TotalAllocationSize() const { return snapshot_->TotalAllocationSize(); }

  void AddPreviousSerializer(FastSerializer* previous) {
    previous_serializers_.push_back(previous);
  }

  struct LabAndOffset {
    LinearAllocationBuffer* lab;
    size_t offset;
  };

  LabAndOffset FoundObject(Tagged<HeapObject> heap_object);

 protected:
  virtual bool is_read_only() { return false; }

  void AddToSnapshot(Tagged<HeapObject> heap_object);

 private:
  void VisitRootPointers(Root root, const char* description,
                         FullObjectSlot start, FullObjectSlot end);

  bool queue_empty() { return queue_.size() == 0; }

  AddressSpace GetAddressSpace(Tagged<HeapObject> object);
  AllocationSpace GetAllocationSpace(Tagged<HeapObject> object);

  // Is the object marked in this or a previous serializer.
  bool IsMarked(Tagged<HeapObject> object);
  // Only ask the current serializer, not previous ones.
  bool LocalIsMarked(Tagged<HeapObject> object);
  // Mark live in this serializer.
  void Mark(Tagged<HeapObject> object, size_t size_in_bytes);

  DISALLOW_GARBAGE_COLLECTION(no_gc_)

  template <typename Slot>
  void VisitSlot(LinearAllocationBuffer* slot_dest_lab, size_t slot_dest_offset,
                 Slot old_slot, Slot new_slot);

  // Create a lab for an object that has a fixed location, ie the same location
  // in the serializer and the deserializer.
  LinearAllocationBuffer* GetOrCreateLabForFixedLocation(
      Tagged<HeapObject> object);

  // Searches this and previous serializers to get the lab for an object.
  LinearAllocationBuffer* GetLabForFixedLocation(Tagged<HeapObject> object);

  // Find a lab in a given space that has space enough for an object.
  LinearAllocationBuffer* GetOrCreatePackedLab(Tagged<HeapObject> object,
                                               size_t size);

  // Looks up in this serializer and the others to find the destination for the
  // object at this address.
  LabAndOffset NewLocation(Address start);

  class ObjectSerializer;

  // For pseudo spaces like roots or tables that are not part of a heap space.
  static constexpr AllocationSpace kIgnoreAllocationSpace = NEW_SPACE;

  Isolate* isolate_;
  AccountingAllocator allocator_;  // For the zone.
  // Used for things that live during serialization, but die once the snapshot
  // is created.
  Zone zone_;
  // The queue contains the gray objects, whose slots have not yet been visited.
  // If it's a reallocating serializer the old location is used.
  ZoneVector<Tagged<HeapObject>> queue_;
  // One bit per word used to mark objects as white (not yet part of the
  // snapshot) or black/gray (have been found).  Objects in the queue are grey,
  // those no longer in the queue are black.  We mark the whole object, so at
  // the end this map shows the words that are in the source lab, but not part
  // of the snapshot.  Manipulated by Mark() and IsMarked().
  // If the location of the object varies between the serializer and the
  // deserializer, this is map is always based on the serializer address.
  ZoneAbslFlatHashMap<Address, uint64_t*> lab_liveness_map_;
  // For FixedLocation snapshots, this lets us find the lab for a given address.
  ZoneAbslFlatHashMap<Address, LinearAllocationBuffer*> lab_map_;
  // For reallocating snapshots this is the forwarding table.  Contains an entry
  // for each object that is marked.
  ZoneAbslFlatHashMap<Address, LabAndOffset> new_locations_;
  // Virtual lab for the roots, in visiting order.
  LinearAllocationBuffer* roots_lab_ = nullptr;
  // For reallocating snapshots, the location of the end of the newest lab.
  size_t address_space_fullnesses_[kNumberOfAddressSpaces];
  LinearAllocationBuffer* lab_per_space_[LAST_SPACE + 1];
  ZoneVector<LinearAllocationBuffer*> all_labs_;
  const PtrComprCageBase cage_base_;
  ExternalReferenceEncoder external_reference_encoder_;
  const Snapshot::SerializerFlags flags_;
  ZoneVector<FastSerializer*> previous_serializers_;

  static int next_lab_index_;

  std::unique_ptr<FastSnapshot> fast_snapshot_;
  FastSnapshot* snapshot_;
  friend class ObjectSerializer;
};

class FastSerializer::ObjectSerializer : public ObjectVisitor {
 public:
  ObjectSerializer(FastSerializer* serializer, Tagged<HeapObject> old_object,
                   Tagged<HeapObject> new_object,
                   LinearAllocationBuffer* dest_lab, size_t dest_offset)
      : serializer_(serializer),
        old_object_(old_object),
        new_object_(new_object),
        dest_lab_(dest_lab) {}
  void SerializeObject();
  void VisitPointers(Tagged<HeapObject> host, ObjectSlot start,
                     ObjectSlot end) override;
  void VisitPointers(Tagged<HeapObject> host, CompressedMaybeObjectSlot start,
                     CompressedMaybeObjectSlot end) override;
  void VisitInstructionStreamPointer(Tagged<Code> host,
                                     InstructionStreamSlot slot) override;
  void VisitEmbeddedPointer(Tagged<InstructionStream> host,
                            RelocInfo* target) override;
  void VisitExternalReference(Tagged<InstructionStream> host,
                              RelocInfo* rinfo) override;
  void VisitInternalReference(Tagged<InstructionStream> host,
                              RelocInfo* rinfo) override;
  void VisitCodeTarget(Tagged<InstructionStream> host,
                       RelocInfo* target) override;
  void VisitOffHeapTarget(Tagged<InstructionStream> host,
                          RelocInfo* target) override {
    UNREACHABLE();
  }
  void VisitExternalPointer(Tagged<HeapObject> host,
                            ExternalPointerSlot slot) override;
  void VisitIndirectPointer(Tagged<HeapObject> host, IndirectPointerSlot slot,
                            IndirectPointerMode mode) override;
  void VisitTrustedPointerTableEntry(Tagged<HeapObject> host,
                                     IndirectPointerSlot slot) override;
  void VisitProtectedPointer(Tagged<TrustedObject> host,
                             ProtectedPointerSlot slot) override;
  void VisitProtectedPointer(Tagged<TrustedObject> host,
                             ProtectedMaybeObjectSlot slot) override;
  void VisitCppHeapPointer(Tagged<HeapObject> host,
                           CppHeapPointerSlot slot) override {
    UNREACHABLE();
  }
  void VisitJSDispatchTableEntry(Tagged<HeapObject> host,
                                 JSDispatchHandle handle) override;
  virtual void VisitMapPointer(Tagged<HeapObject> host) override;

 private:
  FastSerializer* serializer_;
  Tagged<HeapObject> old_object_;  // The object in its original location.
  Tagged<HeapObject> new_object_;  // The object copied into the snapshot.
  LinearAllocationBuffer* dest_lab_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_H_
