// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/wasm

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "wasm/test-backing-store.cc"
#include "wasm/test-c-wasm-entry.cc"
#include "wasm/test-compilation-cache.cc"
#include "wasm/test-grow-memory.cc"
#include "wasm/test-jump-table-assembler.cc"
#include "wasm/test-liftoff-for-fuzzing.cc"
#include "wasm/test-liftoff-inspection.cc"
