// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/builtins
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/builtins/builtins-abstract-module-source.cc"   // nogncheck
#include "src/builtins/builtins-api.cc"                      // nogncheck
#include "src/builtins/builtins-arraybuffer.cc"              // nogncheck
#include "src/builtins/builtins-async-disposable-stack.cc"   // nogncheck
#include "src/builtins/builtins-async-module.cc"             // nogncheck
#include "src/builtins/builtins-atomics-synchronization.cc"  // nogncheck
#include "src/builtins/builtins-bigint.cc"                   // nogncheck
#include "src/builtins/builtins-callsite.cc"                 // nogncheck
#include "src/builtins/builtins-collections.cc"              // nogncheck

#endif  // V8_CLUSTER_BUILD
