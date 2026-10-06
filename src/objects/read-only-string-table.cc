// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/objects/read-only-string-table.h"

#include "src/base/bits.h"
#include "src/execution/isolate.h"
#include "src/handles/handles-inl.h"
#include "src/heap/factory.h"
#include "src/heap/read-only-heap.h"
#include "src/heap/read-only-spaces.h"
#include "src/objects/read-only-string-table-inl.h"

namespace v8 {
namespace internal {

namespace {

// Like OffHeapHashTableBase: at most two thirds full.
bool LoadIsAcceptable(size_t number_of_strings, int capacity) {
  return number_of_strings + number_of_strings / 2 <
         static_cast<size_t>(capacity);
}

int SegmentIndex(uint32_t hash, int num_segments) {
  return (hash >> ReadOnlyStringTable::kSegmentBits) & (num_segments - 1);
}

}  // namespace

// static
bool ReadOnlyStringTable::Contains(Tagged<FixedArray> directory,
                                   Tagged<String> string) {
  if (directory.is_null()) return false;
  const uint32_t hash = string->hash();
  Tagged<WeakFixedArray> segment = SegmentFor(directory, hash);
  const uint32_t mask = segment->length().value() - 1;
  uint32_t index = hash & mask;
  for (uint32_t count = 1;; index = (index + count++) & mask) {
    Tagged<Object> entry = EntryAt(segment, index);
    if (entry == Smi::zero()) return false;
    if (entry == string) return true;
  }
}

// static
bool ReadOnlyStringTable::Fits(
    int num_segments, int segment_length,
    const std::vector<Tagged<InternalizedString>>& strings) {
  std::vector<size_t> counts(num_segments, 0);
  for (Tagged<InternalizedString> string : strings) {
    counts[SegmentIndex(string->EnsureHash(), num_segments)]++;
  }
  for (size_t count : counts) {
    if (!LoadIsAcceptable(count, segment_length)) return false;
  }
  return true;
}

// static
Tagged<FixedArray> ReadOnlyStringTable::Build(
    Isolate* isolate, const std::vector<Tagged<InternalizedString>>& strings) {
  CHECK(isolate->heap()->read_only_space()->writable());
  // Size for the total, then grow for as long as some segment is overloaded.
  // That is rare since the segments share the strings almost evenly.
  int capacity = kMinCapacity;
  while (!LoadIsAcceptable(strings.size(), capacity)) capacity *= 2;
  int segment_length;
  int num_segments;
  for (;; capacity *= 2) {
    segment_length = std::min(capacity, kSegmentSize);
    num_segments = capacity / segment_length;
    if (Fits(num_segments, segment_length, strings)) break;
  }

  // The strings are in read-only space, so the raw pointers survive the
  // allocations below.
  HandleScope scope(isolate);
  Factory* factory = isolate->factory();
  Tagged<FixedArray> directory =
      *factory->NewFixedArray(num_segments, AllocationType::kReadOnly);
  for (int i = 0; i < num_segments; i++) {
    Tagged<WeakFixedArray> segment =
        *factory->NewWeakFixedArray(segment_length, AllocationType::kReadOnly);
    directory->set(i, segment, SKIP_WRITE_BARRIER);
  }
  Fill(directory, strings);
  return directory;
}

// static
void ReadOnlyStringTable::Fill(
    Tagged<FixedArray> directory,
    const std::vector<Tagged<InternalizedString>>& strings) {
  const int num_segments = static_cast<int>(directory->length().value());
  for (int i = 0; i < num_segments; i++) {
    Tagged<WeakFixedArray> segment = SegmentAt(directory, i);
    for (uint32_t j = 0; j < segment->length().value(); j++) {
      segment->set(j, Smi::zero());
    }
  }
  for (Tagged<InternalizedString> string : strings) {
    DCHECK(ReadOnlyHeap::Contains(string));
    const uint32_t hash = string->hash();
    Tagged<WeakFixedArray> segment = SegmentFor(directory, hash);
    const uint32_t mask = segment->length().value() - 1;
    uint32_t index = hash & mask;
    for (uint32_t count = 1; EntryAt(segment, index) != Smi::zero();
         index = (index + count++) & mask) {
      DCHECK_NE(EntryAt(segment, index), string);
    }
    segment->set(index, string, SKIP_WRITE_BARRIER);
  }
}

// static
void ReadOnlyStringTable::CollectEntries(
    Tagged<FixedArray> directory,
    std::vector<Tagged<InternalizedString>>* strings) {
  if (directory.is_null()) return;
  const int num_segments = static_cast<int>(directory->length().value());
  for (int i = 0; i < num_segments; i++) {
    Tagged<WeakFixedArray> segment = SegmentAt(directory, i);
    for (uint32_t j = 0; j < segment->length().value(); j++) {
      Tagged<Object> entry = EntryAt(segment, j);
      if (entry == Smi::zero()) continue;
      strings->push_back(Cast<InternalizedString>(entry));
    }
  }
}

#ifdef VERIFY_HEAP
// static
void ReadOnlyStringTable::Verify(Isolate* isolate,
                                 Tagged<FixedArray> directory) {
  if (directory.is_null()) return;
  CHECK(ReadOnlyHeap::Contains(directory));
  const int num_segments = static_cast<int>(directory->length().value());
  CHECK(base::bits::IsPowerOfTwo(num_segments));
  const uint32_t segment_length = SegmentAt(directory, 0)->length().value();
  CHECK(base::bits::IsPowerOfTwo(segment_length));
  CHECK_IMPLIES(num_segments > 1, segment_length == kSegmentSize);
  for (int i = 0; i < num_segments; i++) {
    Tagged<WeakFixedArray> segment = SegmentAt(directory, i);
    CHECK_EQ(segment->length().value(), segment_length);
    size_t count = 0;
    for (uint32_t j = 0; j < segment_length; j++) {
      Tagged<Object> entry = EntryAt(segment, j);
      if (entry == Smi::zero()) continue;
      count++;
      CHECK(IsInternalizedString(entry));
      Tagged<String> string = Cast<String>(entry);
      CHECK(ReadOnlyHeap::Contains(string));
      CHECK(string->HasHashCode());
      CHECK_EQ(SegmentIndex(string->hash(), num_segments), i);
      CHECK(Contains(directory, string));
    }
    CHECK(LoadIsAcceptable(count, segment_length));
  }
  // The converse, that every internalized string in read-only space is in the
  // table, does not hold: StringTable::LookupKey allocates the internalized
  // copy before taking the lock and drops it if another thread inserted an
  // equal string first, and such a copy allocated while read-only space was
  // writable stays there.
}
#endif  // VERIFY_HEAP

}  // namespace internal
}  // namespace v8
