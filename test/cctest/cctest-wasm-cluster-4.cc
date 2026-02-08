// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/wasm

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "wasm/test-run-wasm-relaxed-simd.cc"
#include "wasm/test-run-wasm-simd-liftoff.cc"
#include "wasm/test-run-wasm-wrappers.cc"
