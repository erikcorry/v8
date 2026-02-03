// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/sandbox
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/sandbox/bytecode-verifier.cc"      // nogncheck
#include "src/sandbox/code-pointer-table.cc"     // nogncheck
#include "src/sandbox/js-dispatch-table.cc"      // nogncheck
#include "src/sandbox/trusted-pointer-scope.cc"  // nogncheck

#endif  // V8_CLUSTER_BUILD
