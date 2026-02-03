// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/heap/paged-spaces.cc"                    // nogncheck
#include "src/heap/pretenuring-handler.cc"             // nogncheck
#include "src/heap/read-only-heap.cc"                  // nogncheck
#include "src/heap/read-only-spaces.cc"                // nogncheck
#include "src/heap/safepoint.cc"                       // nogncheck
#include "src/heap/spaces.cc"                          // nogncheck
#include "src/heap/stress-scavenge-observer.cc"        // nogncheck
#include "src/heap/traced-handles-marking-visitor.cc"  // nogncheck
#include "src/heap/trusted-range.cc"                   // nogncheck
#include "src/heap/zapping.cc"                         // nogncheck

#endif  // V8_CLUSTER_BUILD
