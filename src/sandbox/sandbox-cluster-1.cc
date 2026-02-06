// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/sandbox
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/sandbox/bytecode-verifier.cc"
#include "src/sandbox/code-pointer-table.cc"
#include "src/sandbox/js-dispatch-table.cc"
#include "src/sandbox/trusted-pointer-scope.cc"
