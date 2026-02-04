// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/profiler
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/profiler/allocation-tracker.cc"
#include "src/profiler/strings-storage.cc"
#include "src/profiler/tick-sample.cc"
#include "src/profiler/weak-code-registry.cc"

#endif  // V8_CLUSTER_BUILD
