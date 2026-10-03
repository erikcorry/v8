// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_READ_ONLY_STRING_TABLE_H_
#define V8_OBJECTS_READ_ONLY_STRING_TABLE_H_

#include <vector>

#include "src/common/globals.h"
#include "src/objects/fixed-array.h"
#include "src/objects/string.h"

namespace v8 {
namespace internal {

class Isolate;

// The internalized strings in read-only space, in a hash set that is itself in
// read-only space. It is part of the read-only snapshot image, so deserializing
// it costs nothing and it is shared like the rest of the read-only heap, and
// since its entries never die or move the GC does not look at it. StringTable
// consults it before its own mutable table, which holds the other internalized
// strings. ReadOnlyHeap::string_table() holds the directory, or kNullAddress
// before mksnapshot has built the table and when the snapshot was deserialized
// with rehashing, in which case the read-only strings are in the mutable table
// like any other.
//
// Read-only space cannot hold objects above kMaxRegularHeapObjectSize, so the
// table is a directory array of segments of up to kSegmentSize entries. Each
// segment is an independent hash set: the hash bits above the slot index pick
// the segment, and the quadratic probe sequence of OffHeapHashTableBase stays
// within it, so a lookup consults the directory once. Segment count and length
// are powers of two and all segments have the same length. Empty slots hold
// Smi zero; there are no deleted slots.
class ReadOnlyStringTable final : public AllStatic {
 public:
  // 64KB of entries per segment, so that a segment is a regular-sized object.
  static constexpr int kSegmentBits = 16 - kTaggedSizeLog2;
  static constexpr int kSegmentSize = 1 << kSegmentBits;
  static constexpr int kMinCapacity = 1024;

  // Returns the string matching {key}, or null. {directory} may be null. Safe
  // to call from any thread; the table never changes.
  template <typename IsolateT, typename StringTableKey>
  static inline Tagged<InternalizedString> Lookup(Tagged<FixedArray> directory,
                                                  IsolateT* isolate,
                                                  StringTableKey* key);

  // Whether {string} is an entry of the table.
  static bool Contains(Tagged<FixedArray> directory, Tagged<String> string);

  // Allocates a table holding {strings} in read-only space, which must be
  // writable. Heap allocation must be allowed.
  static Tagged<FixedArray> Build(
      Isolate* isolate, const std::vector<Tagged<InternalizedString>>& strings);

  static void CollectEntries(Tagged<FixedArray> directory,
                             std::vector<Tagged<InternalizedString>>* strings);

#ifdef VERIFY_HEAP
  static void Verify(Isolate* isolate, Tagged<FixedArray> directory);
#endif

 private:
  static inline Tagged<WeakFixedArray> SegmentAt(Tagged<FixedArray> directory,
                                                 uint32_t index);
  static inline Tagged<WeakFixedArray> SegmentFor(Tagged<FixedArray> directory,
                                                  uint32_t hash);
  static inline Tagged<Object> EntryAt(Tagged<WeakFixedArray> segment,
                                       uint32_t index);
  static bool Fits(int num_segments, int segment_length,
                   const std::vector<Tagged<InternalizedString>>& strings);
  static void Fill(Tagged<FixedArray> directory,
                   const std::vector<Tagged<InternalizedString>>& strings);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_OBJECTS_READ_ONLY_STRING_TABLE_H_
