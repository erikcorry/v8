// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/compiler (graph/js)

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/compiler/graph-reducer-unittest.cc"
#include "test/unittests/compiler/graph-trimmer-unittest.cc"
#include "test/unittests/compiler/graph-unittest.cc"
#include "test/unittests/compiler/js-call-reducer-unittest.cc"
#include "test/unittests/compiler/js-create-lowering-unittest.cc"
#include "test/unittests/compiler/js-intrinsic-lowering-unittest.cc"
#include "test/unittests/compiler/js-native-context-specialization-unittest.cc"
#include "test/unittests/compiler/js-operator-unittest.cc"
#include "test/unittests/compiler/js-typed-lowering-unittest.cc"
