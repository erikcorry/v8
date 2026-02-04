// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/builtins
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/builtins/builtins-console.cc"
#include "src/builtins/builtins-dataview.cc"
#include "src/builtins/builtins-disposable-stack.cc"
#include "src/builtins/builtins-error.cc"
#include "src/builtins/builtins-internal.cc"
#include "src/builtins/builtins-json.cc"
#include "src/builtins/builtins-math.cc"
#include "src/builtins/builtins-number.cc"
#include "src/builtins/builtins-object.cc"

#endif  // V8_CLUSTER_BUILD
