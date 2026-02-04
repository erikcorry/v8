// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/heap/memory-balancer.cc"  // nogncheck
#include "src/heap/memory-chunk.cc"     // nogncheck
#include "src/heap/memory-reducer.cc"   // nogncheck
#include "src/heap/minor-gc-job.cc"     // nogncheck
#include "src/heap/mutable-page.cc"     // nogncheck
#include "src/heap/new-spaces.cc"       // nogncheck
#include "src/heap/normal-page.cc"      // nogncheck

#endif  // V8_CLUSTER_BUILD
