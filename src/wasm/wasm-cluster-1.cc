// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/wasm
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/wasm/canonical-types.cc"
#include "src/wasm/code-space-access.cc"
#include "src/wasm/constant-expression-interface.cc"
#include "src/wasm/jump-table-assembler.cc"
#include "src/wasm/sync-streaming-decoder.cc"
#include "src/wasm/wasm-code-pointer-table.cc"
#include "src/wasm/wasm-export-wrapper-cache.cc"
#include "src/wasm/wasm-external-refs.cc"
#include "src/wasm/wasm-features.cc"
#include "src/wasm/wasm-result.cc"
