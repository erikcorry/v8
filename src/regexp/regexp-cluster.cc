// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/regexp
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/regexp/regexp-bytecode-iterator.cc"  // nogncheck
#include "src/regexp/regexp-macro-assembler.cc"    // nogncheck
#include "src/regexp/regexp-utils.cc"              // nogncheck

#endif  // V8_CLUSTER_BUILD
