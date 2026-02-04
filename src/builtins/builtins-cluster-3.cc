// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/builtins
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/builtins/builtins-reflect.cc"
#include "src/builtins/builtins-regexp.cc"
#include "src/builtins/builtins-shadow-realm.cc"
#include "src/builtins/builtins-shared-array.cc"
#include "src/builtins/builtins-sharedarraybuffer.cc"
#include "src/builtins/builtins-string.cc"
#include "src/builtins/builtins-symbol.cc"
#include "src/builtins/builtins-weak-refs.cc"
#include "src/builtins/constants-table-builder.cc"

#endif  // V8_CLUSTER_BUILD
