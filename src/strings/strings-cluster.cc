// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/strings
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/strings/string-builder.cc"
#include "src/strings/string-hasher.cc"
#include "src/strings/string-stream.cc"

#endif  // V8_CLUSTER_BUILD
