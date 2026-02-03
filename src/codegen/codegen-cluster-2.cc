// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/codegen
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/codegen/jump-table-info.cc"               // nogncheck
#include "src/codegen/macro-assembler-base.cc"          // nogncheck
#include "src/codegen/maglev-safepoint-table.cc"        // nogncheck
#include "src/codegen/pending-optimization-table.cc"    // nogncheck
#include "src/codegen/source-position-table.cc"         // nogncheck
#include "src/codegen/source-position.cc"               // nogncheck
#include "src/codegen/unoptimized-compilation-info.cc"  // nogncheck

#endif  // V8_CLUSTER_BUILD
