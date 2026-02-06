// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/heap/heap-controller.cc"
#include "src/heap/heap-layout-tracer.cc"
#include "src/heap/heap-layout.cc"
#include "src/heap/incremental-marking-job.cc"
#include "src/heap/large-spaces.cc"
#include "src/heap/local-factory.cc"
#include "src/heap/local-heap.cc"
#include "src/heap/mark-sweep-utilities.cc"
#include "src/heap/marking-worklist.cc"
#include "src/heap/marking.cc"
#include "src/heap/memory-allocator.cc"
