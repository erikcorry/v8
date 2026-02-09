// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/compiler (node/regalloc)

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/compiler/node-cache-unittest.cc"
#include "test/unittests/compiler/node-matchers-unittest.cc"
#include "test/unittests/compiler/node-properties-unittest.cc"
#include "test/unittests/compiler/node-test-utils.cc"
#include "test/unittests/compiler/node-unittest.cc"
#include "test/unittests/compiler/opcodes-unittest.cc"
#include "test/unittests/compiler/persistent-unittest.cc"
#include "test/unittests/compiler/redundancy-elimination-unittest.cc"
#include "test/unittests/compiler/regalloc/live-range-unittest.cc"
#include "test/unittests/compiler/regalloc/move-optimizer-unittest.cc"
#include "test/unittests/compiler/regalloc/register-allocator-unittest.cc"
