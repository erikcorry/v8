// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/compiler

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/cctest/compiler/test-js-constant-cache.cc"
#include "test/cctest/compiler/test-js-context-specialization.cc"
#include "test/cctest/compiler/test-js-typed-lowering.cc"
#include "test/cctest/compiler/test-jump-threading.cc"
#include "test/cctest/compiler/test-loop-analysis.cc"
#include "test/cctest/compiler/test-machine-operator-reducer.cc"
