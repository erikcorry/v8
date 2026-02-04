// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/heap/array-buffer-sweeper.cc"                // nogncheck
#include "src/heap/base-page.cc"                           // nogncheck
#include "src/heap/code-range.cc"                          // nogncheck
#include "src/heap/code-stats.cc"                          // nogncheck
#include "src/heap/collection-barrier.cc"                  // nogncheck
#include "src/heap/combined-heap.cc"                       // nogncheck
#include "src/heap/ephemeron-remembered-set.cc"            // nogncheck
#include "src/heap/evacuation-allocator.cc"                // nogncheck
#include "src/heap/finalization-registry-cleanup-task.cc"  // nogncheck
#include "src/heap/free-list.cc"                           // nogncheck
#include "src/heap/heap-allocator.cc"                      // nogncheck

#endif  // V8_CLUSTER_BUILD
