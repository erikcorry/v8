// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/codegen
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/codegen/code-comments.cc"
#include "src/codegen/code-desc.cc"
#include "src/codegen/code-factory.cc"
#include "src/codegen/code-reference.cc"
#include "src/codegen/compilation-cache.cc"
#include "src/codegen/handler-table.cc"
