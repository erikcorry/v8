// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/regexp
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/regexp/regexp-bytecode-iterator.cc"
#include "src/regexp/regexp-macro-assembler.cc"
#include "src/regexp/regexp-utils.cc"

#endif  // V8_CLUSTER_BUILD
