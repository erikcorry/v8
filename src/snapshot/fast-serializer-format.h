// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_FAST_SERIALIZER_FORMAT_H_
#define V8_SNAPSHOT_FAST_SERIALIZER_FORMAT_H_

#include "src/common/globals.h"
#include "src/snapshot/fast-serializer.h"

namespace v8 {
namespace internal {

// overlay struct
// this is at the head of the mmapped file
struct SerializedSnapshotHeader {
  enum SectionOffset : uint8_t {
    kSectionHeader,
    kSectionLabs,
    // kSectionEmbedderData,
    kSectionRelocs,
    kSectionData,
    kSectionCount
  };
  struct SectionDescriptor {
    uint32_t offset;
    uint32_t length;
  };
  uint64_t magic;
  SectionDescriptor section_descs[kSectionCount];
};

// overlay struct
struct SerializedLabEntry {
  // TODO maybe don't need this as everything is relative or could be global
  // original address
  Address lab_start;
  // this is implicit but it's free because of padding so use it for checks
  uint16_t lab_offset;
  uint8_t padding;
  uint8_t space_type;
  // actual lab data - simd aligned
  uint32_t data_start_offset;
  uint32_t data_length;
  // relocation entries
  uint32_t relocation_entry_start;
  uint32_t relocation_entry_count;
};

struct SerializedRelocEntry {
  uint32_t target_lab_index;
  uint32_t offset_in_source;
  uint8_t reloc_type;
};

// overlay struct
struct SerializedLabSection {
  uint32_t n_labs;
  SerializedLabEntry labs[];
};

// overlay struct
struct EmbedderDataSection {
  // TBD
  uint64_t padding;
};

class SerializedSnapshot {
 public:
  SerializedSnapshot(size_t data_length, size_t alignment);
  SerializedSnapshot(std::unique_ptr<uint8_t[]> data, size_t data_length);

  const uint8_t* data() const { return data_.get(); }

  size_t data_length() { return data_length_; }

 private:
  std::unique_ptr<uint8_t[]> data_;
  size_t data_length_;
};

class FileOffsetCalculator {
 public:
  FileOffsetCalculator() : current_(0) {}
  explicit FileOffsetCalculator(uint32_t current) : current_(current) {}
  explicit FileOffsetCalculator(size_t current)
      : current_(static_cast<uint32_t>(current)) {}

  uint32_t current() const { return current_; }

  uint32_t Allocate(size_t bytes, size_t alignment) {
    current_ = RoundUp(current_, alignment);
    auto start = current_;
    current_ += bytes;
    return start;
  }

  uint32_t AllocateArray(size_t n, size_t bytes, size_t alignment) {
    return Allocate(n * bytes, alignment);
  }

 private:
  uint32_t current_;
};

class SerializedSnapshotSizeCalculator {
 public:
  SerializedSnapshotSizeCalculator(
      const ZoneVector<LinearAllocationBuffer*>& labs,
      const ZoneVector<Relocation>& relocations);

  inline SerializedSnapshotHeader::SectionDescriptor GetDescriptor(
      SerializedSnapshotHeader::SectionOffset offset) {
    DCHECK_LT(offset, SerializedSnapshotHeader::kSectionCount);
    return section_descs_[offset];
  }

  inline size_t GetTotalSize() const { return total_size_; }

  struct LabRelocationInfo {
    uint32_t offset;
    uint32_t count;
  };

  inline LabRelocationInfo GetLabRelocationInfo(size_t lab_index) const {
    return lab_reloc_info_[lab_index];
  }

  uint32_t GetLabDataOffset(size_t lab_index) const;

  inline size_t GetLabCount() const { return labs_.size(); }

  inline const ZoneVector<LinearAllocationBuffer*>& labs() const {
    return labs_;
  }
  inline const ZoneVector<Relocation>& relocations() const {
    return relocations_;
  }

  inline const ZoneMap<uint16_t, ZoneVector<const Relocation*>>&
  relocations_by_lab() const {
    return relocations_by_lab_;
  }
  inline const ZoneVector<LabRelocationInfo>& lab_reloc_info() const {
    return lab_reloc_info_;
  }
  inline const ZoneVector<uint32_t>& lab_data_offsets() const {
    return lab_data_offsets_;
  }

 private:
  void CalculateTotalSize();

  FileOffsetCalculator CalculateHeaderSize(FileOffsetCalculator offset);
  FileOffsetCalculator CalculateLabMetadataSize(FileOffsetCalculator offset);
  FileOffsetCalculator CalculateRelocationDataSize(FileOffsetCalculator offset);
  FileOffsetCalculator CalculateDataSectionSize(FileOffsetCalculator offset);

  void GroupRelocationsByLab();

  const ZoneVector<LinearAllocationBuffer*>& labs_;
  const ZoneVector<Relocation>& relocations_;

  ZoneMap<uint16_t, ZoneVector<const Relocation*>> relocations_by_lab_;
  ZoneVector<LabRelocationInfo> lab_reloc_info_;
  ZoneVector<uint32_t> lab_data_offsets_;

  SerializedSnapshotHeader::SectionDescriptor
      section_descs_[SerializedSnapshotHeader::kSectionCount];
  uint32_t total_size_;
};

class FastSnapshotSerializer {
 public:
  FastSnapshotSerializer(ZoneVector<LinearAllocationBuffer*> labs,
                         ZoneVector<Relocation> relocations);

  SerializedSnapshot Serialize();

 private:
  uintptr_t GetPointerAt(size_t offset);

  template <typename T>
  inline T* GetPointerAtTyped(size_t offset) {
    uintptr_t ptr = GetPointerAt(offset);
    DCHECK(IsAligned(ptr, alignof(T)));
    return reinterpret_cast<T*>(ptr);
  }
  void WriteHeader();
  void WriteLabMetadata();
  void WriteRelocationData();
  void WriteDataSection();

  SerializedSnapshotSizeCalculator calculator_;
  SerializedSnapshot snapshot_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_FAST_SERIALIZER_FORMAT_H_
