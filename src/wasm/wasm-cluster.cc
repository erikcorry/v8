// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/wasm
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/wasm/canonical-types.cc"                // nogncheck
#include "src/wasm/code-space-access.cc"              // nogncheck
#include "src/wasm/constant-expression-interface.cc"  // nogncheck
#include "src/wasm/jump-table-assembler.cc"           // nogncheck
#include "src/wasm/sync-streaming-decoder.cc"         // nogncheck
#include "src/wasm/wasm-code-pointer-table.cc"        // nogncheck
#include "src/wasm/wasm-export-wrapper-cache.cc"      // nogncheck
#include "src/wasm/wasm-external-refs.cc"             // nogncheck
#include "src/wasm/wasm-features.cc"                  // nogncheck
#include "src/wasm/wasm-result.cc"                    // nogncheck

#endif  // V8_CLUSTER_BUILD
