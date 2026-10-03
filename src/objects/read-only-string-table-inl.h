// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_READ_ONLY_STRING_TABLE_INL_H_
#define V8_OBJECTS_READ_ONLY_STRING_TABLE_INL_H_

#include "src/objects/read-only-string-table.h"
// Include the non-inl header before the rest of the headers.

#include "src/objects/fixed-array-inl.h"
#include "src/objects/string-inl.h"
#include "src/objects/string-table.h"

namespace v8 {
namespace internal {

// static
Tagged<WeakFixedArray> ReadOnlyStringTable::SegmentAt(
    Tagged<FixedArray> directory, uint32_t index) {
  return Cast<WeakFixedArray>(directory->get(index));
}

// static
Tagged<WeakFixedArray> ReadOnlyStringTable::SegmentFor(
    Tagged<FixedArray> directory, uint32_t hash) {
  // With a single segment its length may be below kSegmentSize, and the
  // segment index is then zero regardless of the shift.
  return SegmentAt(directory,
                   (hash >> kSegmentBits) & (directory->length().value() - 1));
}

// static
Tagged<Object> ReadOnlyStringTable::EntryAt(Tagged<WeakFixedArray> segment,
                                            uint32_t index) {
  Tagged<MaybeObject> entry = segment->get(index);
  if (entry.IsSmi()) return entry.ToSmi();
  return entry.GetHeapObjectAssumeStrong();
}

// static
template <typename IsolateT, typename StringTableKey>
Tagged<InternalizedString> ReadOnlyStringTable::Lookup(
    Tagged<FixedArray> directory, IsolateT* isolate, StringTableKey* key) {
  if (directory.is_null()) return {};
  const uint32_t hash = key->hash();
  Tagged<WeakFixedArray> segment = SegmentFor(directory, hash);
  const uint32_t mask = segment->length().value() - 1;
  uint32_t index = hash & mask;
  for (uint32_t count = 1;; index = (index + count++) & mask) {
    Tagged<Object> entry = EntryAt(segment, index);
    if (entry == Smi::zero()) return {};
    Tagged<String> string = Cast<String>(entry);
    if (string->hash() == hash && string->length() == key->length() &&
        key->IsMatch(isolate, string)) {
      return Cast<InternalizedString>(string);
    }
  }
}

}  // namespace internal
}  // namespace v8

#endif  // V8_OBJECTS_READ_ONLY_STRING_TABLE_INL_H_
