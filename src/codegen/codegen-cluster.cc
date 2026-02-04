// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/codegen
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/codegen/code-comments.cc"      // nogncheck
#include "src/codegen/code-desc.cc"          // nogncheck
#include "src/codegen/code-factory.cc"       // nogncheck
#include "src/codegen/code-reference.cc"     // nogncheck
#include "src/codegen/compilation-cache.cc"  // nogncheck
#include "src/codegen/handler-table.cc"      // nogncheck

#endif  // V8_CLUSTER_BUILD
