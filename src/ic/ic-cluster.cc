// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/ic
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/ic/call-optimization.cc"      // nogncheck
#include "src/ic/handler-configuration.cc"  // nogncheck
#include "src/ic/ic-stats.cc"               // nogncheck
#include "src/ic/stub-cache.cc"             // nogncheck

#endif  // V8_CLUSTER_BUILD
