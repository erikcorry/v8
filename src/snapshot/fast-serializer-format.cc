// Copyright 2025 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/snapshot/fast-serializer-format.h"

#include "src/snapshot/fast-serializer-deserializer.h"

namespace v8 {
namespace internal {

// TODO find a good number for this depending on arch
// 64 bytes ought to be enough for everyone
static const size_t kSimdAlignment = 64;

uint32_t FileOffsetCalculator::Allocate(size_t bytes, size_t alignment) {
  current_ = RoundUp(current_, alignment);
  auto start = current_;
  current_ += bytes;
  return start;
}

SerializedSnapshotSizeCalculator::SerializedSnapshotSizeCalculator(
    const ZoneVector<LinearAllocationBuffer*>& labs,
    const ZoneVector<Relocation>& relocations)
    : labs_(labs),
      relocations_(relocations),
      relocations_by_lab_(
          ZoneMap<uint16_t, ZoneVector<const Relocation*>>(labs.zone())),
      lab_reloc_info_(ZoneVector<LabRelocationInfo>(labs.zone())),
      lab_data_offsets_(ZoneVector<uint32_t>(labs.zone())) {
  lab_reloc_info_.resize(labs_.size(), {0, 0});
  lab_data_offsets_.resize(labs_.size(), 0);
  // TODO this should already have been done but double check.
  for (size_t i = 0; i < labs_.size(); ++i) {
    CHECK_EQ(labs_[i]->index(), i);
  }
  // Generate helper data.
  GroupRelocationsByLab();
  // Compute everything.
  CalculateTotalSize();
}

void SerializedSnapshotSizeCalculator::GroupRelocationsByLab() {
  for (const auto& reloc : relocations_) {
    size_t source_lab = reloc.source_lab();
    auto it = relocations_by_lab_.find(source_lab);
    if (it == relocations_by_lab_.end()) {
      it =
          relocations_by_lab_
              .insert({source_lab, ZoneVector<const Relocation*>(labs_.zone())})
              .first;
    }
    it->second.push_back(&reloc);
  }
  // Now sort relocations.
  // TODO - are these already sorted and this is skippable?
  for (auto& relocs : relocations_by_lab_) {
    std::sort(relocs.second.begin(), relocs.second.end(),
              [](const Relocation* a, const Relocation* b) {
                return a->offset() < b->offset();
              });
  }
}

FileOffsetCalculator SerializedSnapshotSizeCalculator::CalculateHeaderSize(
    FileOffsetCalculator offset) {
  offset.Allocate<SerializedSnapshotHeader>();
  return offset;
}

FileOffsetCalculator SerializedSnapshotSizeCalculator::CalculateLabMetadataSize(
    FileOffsetCalculator offset) {
  // Header
  offset.Allocate<SerializedLabSection>();
  // Array of labs.
  offset.AllocateArray<SerializedLabEntry>(labs_.size());
  return offset;
}

FileOffsetCalculator
SerializedSnapshotSizeCalculator::CalculateRelocationDataSize(
    FileOffsetCalculator offset) {
  // Process each lab's relocations.
  for (size_t lab_idx = 0; lab_idx < labs_.size(); ++lab_idx) {
    auto it = relocations_by_lab_.find(lab_idx);
    if (it == relocations_by_lab_.end()) {
      // No relocations for this lab.
      lab_reloc_info_[lab_idx].offset = 0;
      lab_reloc_info_[lab_idx].count = 0;
      continue;
    }
    const auto& relocs = it->second;
    size_t reloc_count = relocs.size();
    // Record offset and count for this lab.
    lab_reloc_info_[lab_idx].offset =
        offset.AllocateArray<SerializedRelocEntry>(reloc_count);
    lab_reloc_info_[lab_idx].count = static_cast<uint32_t>(reloc_count);
  }
  return offset;
}

FileOffsetCalculator SerializedSnapshotSizeCalculator::CalculateDataSectionSize(
    FileOffsetCalculator offset) {
  // Process each lab's data
  for (size_t lab_idx = 0; lab_idx < labs_.size(); ++lab_idx) {
    const auto& lab = labs_[lab_idx];
    size_t lab_size = lab->highest() - lab->lowest();
    // Record offset for this lab's data
    lab_data_offsets_[lab_idx] = offset.Allocate(lab_size, kSimdAlignment);
  }
  return offset;
}

void SerializedSnapshotSizeCalculator::CalculateTotalSize() {
  FileOffsetCalculator offset;
  // Header section.
  static_assert(SerializedSnapshotHeader::kSectionHeader == 0);
  {
    uint32_t section_start = offset.current();
    offset = CalculateHeaderSize(offset);
    section_descs_[SerializedSnapshotHeader::kSectionHeader] = {
        section_start, offset.current() - section_start};
  }
  // Lab section.
  static_assert(SerializedSnapshotHeader::kSectionLabs == 1);
  {
    uint32_t section_start = offset.current();
    offset = CalculateLabMetadataSize(offset);
    section_descs_[SerializedSnapshotHeader::kSectionLabs] = {
        section_start, offset.current() - section_start};
  }
  // Reloc section.
  static_assert(SerializedSnapshotHeader::kSectionRelocs == 2);
  {
    uint32_t section_start = offset.current();
    offset = CalculateRelocationDataSize(offset);
    section_descs_[SerializedSnapshotHeader::kSectionRelocs] = {
        section_start, offset.current() - section_start};
  }
  // Data section.
  static_assert(SerializedSnapshotHeader::kSectionData == 3);
  {
    uint32_t section_start = offset.current();
    offset = CalculateDataSectionSize(offset);
    section_descs_[SerializedSnapshotHeader::kSectionData] = {
        section_start, offset.current() - section_start};
  }
  total_size_ = offset.current();
}

SerializedSnapshot::SerializedSnapshot(size_t data_length, size_t alignment)
    : data_(std::unique_ptr<const uint8_t>(
          static_cast<uint8_t*>(aligned_alloc(alignment, data_length)))),
      data_length_(data_length) {}

SerializedSnapshot::SerializedSnapshot(std::unique_ptr<const uint8_t> data,
                                       size_t data_length)
    : data_(std::move(data)), data_length_(data_length) {
  DCHECK(IsAligned(reinterpret_cast<size_t>(data.get()), kSimdAlignment));
}

FastSnapshotSerializer::FastSnapshotSerializer(
    ZoneVector<LinearAllocationBuffer*> labs,
    ZoneVector<Relocation> relocations)
    : calculator_(SerializedSnapshotSizeCalculator(labs, relocations)),
      snapshot_(
          SerializedSnapshot(calculator_.GetTotalSize(), kSimdAlignment)) {}

uintptr_t FastSnapshotSerializer::GetPointerAt(size_t offset) {
  return reinterpret_cast<uintptr_t>(&snapshot_.data()[offset]);
}

// V8LABS in little endian ascii.
constexpr uint64_t MAGIC_NUMBER = 0x5342414C38560000ULL;

void FastSnapshotSerializer::WriteHeader() {
  auto offset =
      calculator_.GetDescriptor(SerializedSnapshotHeader::kSectionHeader)
          .offset;
  auto* section = GetPointerAtTyped<SerializedSnapshotHeader>(offset);
  section->magic = MAGIC_NUMBER;
  for (uint8_t i = 0; i < SerializedSnapshotHeader::kSectionCount; ++i) {
    section->section_descs[i] = calculator_.GetDescriptor(
        static_cast<SerializedSnapshotHeader::SectionOffset>(i));
  }
}

void FastSnapshotSerializer::WriteLabMetadata() {
  FileOffsetCalculator offset(
      calculator_.GetDescriptor(SerializedSnapshotHeader::kSectionLabs).offset);
  // Header.
  auto* section = GetPointerAtTyped<SerializedLabSection>(offset.current());
  const auto& labs = calculator_.labs();
  section->n_labs = static_cast<uint32_t>(calculator_.labs().size());
  // Lab array.
  for (size_t lab_idx = 0; lab_idx < labs.size(); ++lab_idx) {
    const auto& lab = labs[lab_idx];
    DCHECK_EQ(lab->index(), lab_idx);
    // TODO check the labs[] field in SerializedLabSection has the correct
    // offset
    SerializedLabEntry* entry = GetPointerAtTyped<SerializedLabEntry>(
        offset.Allocate<SerializedLabEntry>());
    entry->lab_start = lab->start();
    entry->lab_offset = static_cast<uint32_t>(lab_idx);
    entry->padding = 0;
    entry->space_type = lab->space();
    entry->data_start_offset = calculator_.GetLabDataOffset(lab_idx);
    entry->data_length = static_cast<uint32_t>(lab->highest() - lab->lowest());
    auto reloc_info = calculator_.GetLabRelocationInfo(lab_idx);
    entry->relocation_entry_start = reloc_info.offset;
    entry->relocation_entry_count = static_cast<uint32_t>(reloc_info.count);
  }
}

void FastSnapshotSerializer::WriteRelocationData() {
  FileOffsetCalculator offset(
      calculator_.GetDescriptor(SerializedSnapshotHeader::kSectionRelocs)
          .offset);
  const auto& relocations_by_lab = calculator_.relocations_by_lab();
  const auto& labs = calculator_.labs();
  for (size_t lab_idx = 0; lab_idx < labs.size(); ++lab_idx) {
    auto it = relocations_by_lab.find(lab_idx);
    if (it == relocations_by_lab.end()) {
      continue;
    }
    const auto& relocs = it->second;
    // TODO use ordering from calculator
    auto reloc_info = calculator_.GetLabRelocationInfo(lab_idx);
    DCHECK_EQ(reloc_info.offset, offset.current());
    DCHECK_EQ(reloc_info.count, relocs.size());
    for (size_t i = 0; i < relocs.size(); ++i) {
      const Relocation* reloc = relocs[i];
      SerializedRelocEntry* entry = GetPointerAtTyped<SerializedRelocEntry>(
          offset.Allocate<SerializedRelocEntry>());
      // Fill in relocation data.
      entry->target_lab_index = static_cast<uint32_t>(reloc->destination_lab());
      entry->offset_in_source = static_cast<uint32_t>(reloc->offset());
      // TODO do we only need 2 types of relocations?
      // TODO make an enum
      if (reloc->compressed()) {
        entry->reloc_type = 0x1;
      } else {
        entry->reloc_type = 0x0;
      }
    }
  }
}

void FastSnapshotSerializer::WriteDataSection() {
  const auto& labs = calculator_.labs();
  for (size_t lab_idx = 0; lab_idx < labs.size(); ++lab_idx) {
    const auto& lab = labs[lab_idx];
    size_t data_offset = calculator_.GetLabDataOffset(lab_idx);
    DCHECK(IsAligned(data_offset, kSimdAlignment));
    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(GetPointerAt(data_offset));
    size_t lab_size = lab->highest() - lab->lowest();
    std::memcpy(data_ptr, lab->BackingAt(0), lab_size);
  }
}

}  // namespace internal
}  // namespace v8
