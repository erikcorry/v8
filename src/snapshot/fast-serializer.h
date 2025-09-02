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
#include "src/snapshot/fast-serializer-deserializer.h"
#include "src/snapshot/snapshot.h"
#include "src/utils/identity-map.h"

namespace v8 {
namespace internal {

// The 'fast' refers to the speed of deserialization.  The serializer
// itself is not particularly fast.
class FastSerializer {
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

  void VisitRootPointers(Root root, const char* description,
                         FullObjectSlot start, FullObjectSlot end);
  void SerializeRootObject(FullObjectSlot slot);

  bool queue_empty() { return queue_.size() == 0; }

  bool IsMarked(Tagged<HeapObject> object);
  void Mark(Tagged<HeapObject> object, size_t size_in_bytes);

  std::unique_ptr<FastSnapshot> Run();

 private:
  DISALLOW_GARBAGE_COLLECTION(no_gc_)

  void VisitUncompressedSlot(size_t source_lab, Tagged<Object> slot_contents);
  void VisitCompressedSlot(size_t source_lab, size_t destination_lab,
                           Address slot_address);

  LinearAllocationBuffer* GetOrCreateLab(Address address);

  class ObjectSerializer;

  Isolate* isolate_;
  AccountingAllocator allocator_;  // For the zone.
  // Used for things that live during serialization, but die once the snapshot
  // is created.
  Zone zone_;
  SmallZoneVector<Tagged<HeapObject>, 10> queue_;
  // One bit per word used to mark objects as white (not yet part of the
  // snapshot) or black/gray (have been found).  Objects in the queue are grey,
  // those no longer in the queue are black.  We mark the whole object, so at
  // the end this map shows the words that are in the source lab, but not part
  // of the snapshot.  Manipulated by Mark() and IsMarked().
  ZoneAbslFlatHashMap<Address, uint64_t*> lab_liveness_map_;
  const PtrComprCageBase cage_base_;
  ExternalReferenceEncoder external_reference_encoder_;
  const Snapshot::SerializerFlags flags_;

  std::unique_ptr<FastSnapshot> fast_snapshot_;
  FastSnapshot* snapshot_;
  friend class ObjectSerializer;
};

class FastSerializer::ObjectSerializer : public ObjectVisitor {
 public:
  ObjectSerializer(FastSerializer* serializer)
      : isolate_(serializer->isolate_), serializer_(serializer) {}
  void SerializeObject();
  void VisitPointers(Tagged<HeapObject> host, ObjectSlot start,
                     ObjectSlot end) override;
  void VisitPointers(Tagged<HeapObject> host, MaybeObjectSlot start,
                     MaybeObjectSlot end) override;
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

 private:
  Isolate* isolate_;
  FastSerializer* serializer_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_H_
