// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/compiler

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/cctest/compiler/codegen-tester.cc"
#include "test/cctest/compiler/function-tester.cc"
#include "test/cctest/compiler/test-atomic-load-store-codegen.cc"
#include "test/cctest/compiler/test-basic-block-profiler.cc"
#include "test/cctest/compiler/test-branch-combine.cc"
#include "test/cctest/compiler/test-calls-with-arraylike-or-spread.cc"
#include "test/cctest/compiler/test-code-assembler.cc"
