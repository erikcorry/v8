// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/execution
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/execution/embedder-state.cc"
#include "src/execution/futex-emulation.cc"
#include "src/execution/microtask-queue.cc"
#include "src/execution/protectors.cc"
#include "src/execution/stack-guard.cc"
#include "src/execution/tiering-manager.cc"
#include "src/execution/v8threads.cc"

#endif  // V8_CLUSTER_BUILD
