// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/builtins
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/builtins/builtins-abstract-module-source.cc"
#include "src/builtins/builtins-api.cc"
#include "src/builtins/builtins-arraybuffer.cc"
#include "src/builtins/builtins-async-disposable-stack.cc"
#include "src/builtins/builtins-async-module.cc"
#include "src/builtins/builtins-atomics-synchronization.cc"
#include "src/builtins/builtins-bigint.cc"
#include "src/builtins/builtins-callsite.cc"
#include "src/builtins/builtins-collections.cc"

#endif  // V8_CLUSTER_BUILD
