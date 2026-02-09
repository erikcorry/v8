// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/wasm

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/cctest/wasm/test-backing-store.cc"
#include "test/cctest/wasm/test-c-wasm-entry.cc"
#include "test/cctest/wasm/test-compilation-cache.cc"
#include "test/cctest/wasm/test-grow-memory.cc"
#include "test/cctest/wasm/test-jump-table-assembler.cc"
#include "test/cctest/wasm/test-liftoff-for-fuzzing.cc"
#include "test/cctest/wasm/test-liftoff-inspection.cc"
