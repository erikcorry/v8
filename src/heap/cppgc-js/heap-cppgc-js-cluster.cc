// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/heap/cppgc-js
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/heap/cppgc-js/cross-heap-remembered-set.cc"      // nogncheck
#include "src/heap/cppgc-js/unified-heap-marking-state.cc"     // nogncheck
#include "src/heap/cppgc-js/unified-heap-marking-verifier.cc"  // nogncheck
#include "src/heap/cppgc-js/unified-heap-marking-visitor.cc"   // nogncheck

#endif  // V8_CLUSTER_BUILD
