// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/heap/array-buffer-sweeper.cc"
#include "src/heap/base-page.cc"
#include "src/heap/code-range.cc"
#include "src/heap/code-stats.cc"
#include "src/heap/collection-barrier.cc"
#include "src/heap/combined-heap.cc"
#include "src/heap/ephemeron-remembered-set.cc"
#include "src/heap/evacuation-allocator.cc"
#include "src/heap/finalization-registry-cleanup-task.cc"
#include "src/heap/free-list.cc"
#include "src/heap/heap-allocator.cc"
