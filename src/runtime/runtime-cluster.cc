// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/runtime
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/runtime/runtime-array.cc"
#include "src/runtime/runtime-atomics.cc"
#include "src/runtime/runtime-bigint.cc"
#include "src/runtime/runtime-collections.cc"
#include "src/runtime/runtime-date.cc"
#include "src/runtime/runtime-forin.cc"
#include "src/runtime/runtime-function.cc"
#include "src/runtime/runtime-futex.cc"
#include "src/runtime/runtime-generator.cc"
#include "src/runtime/runtime-intl.cc"
#include "src/runtime/runtime-module.cc"

#endif  // V8_CLUSTER_BUILD
