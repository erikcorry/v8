// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/heap/paged-spaces.cc"
#include "src/heap/pretenuring-handler.cc"
#include "src/heap/read-only-heap.cc"
#include "src/heap/read-only-spaces.cc"
#include "src/heap/safepoint.cc"
#include "src/heap/spaces.cc"
#include "src/heap/stress-scavenge-observer.cc"
#include "src/heap/traced-handles-marking-visitor.cc"
#include "src/heap/trusted-range.cc"
#include "src/heap/zapping.cc"

#endif  // V8_CLUSTER_BUILD
