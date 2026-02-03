// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/heap/heap-controller.cc"          // nogncheck
#include "src/heap/heap-layout-tracer.cc"       // nogncheck
#include "src/heap/heap-layout.cc"              // nogncheck
#include "src/heap/incremental-marking-job.cc"  // nogncheck
#include "src/heap/large-spaces.cc"             // nogncheck
#include "src/heap/local-factory.cc"            // nogncheck
#include "src/heap/local-heap.cc"               // nogncheck
#include "src/heap/mark-sweep-utilities.cc"     // nogncheck
#include "src/heap/marking-worklist.cc"         // nogncheck
#include "src/heap/marking.cc"                  // nogncheck
#include "src/heap/memory-allocator.cc"         // nogncheck

#endif  // V8_CLUSTER_BUILD
