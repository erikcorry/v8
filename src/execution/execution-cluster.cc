// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/execution
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/execution/embedder-state.cc"   // nogncheck
#include "src/execution/futex-emulation.cc"  // nogncheck
#include "src/execution/microtask-queue.cc"  // nogncheck
#include "src/execution/protectors.cc"       // nogncheck
#include "src/execution/stack-guard.cc"      // nogncheck
#include "src/execution/tiering-manager.cc"  // nogncheck
#include "src/execution/v8threads.cc"        // nogncheck

#endif  // V8_CLUSTER_BUILD
