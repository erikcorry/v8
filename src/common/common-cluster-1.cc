// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/common
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/common/assert-scope.cc"
#include "src/common/code-memory-access.cc"
