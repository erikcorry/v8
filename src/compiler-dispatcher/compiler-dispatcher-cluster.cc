// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/compiler-dispatcher
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/compiler-dispatcher/lazy-compile-dispatcher.cc"  // nogncheck
#include "src/compiler-dispatcher/optimizing-compile-dispatcher.cc"  // nogncheck

#endif  // V8_CLUSTER_BUILD
