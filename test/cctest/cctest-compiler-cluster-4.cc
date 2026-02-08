// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/compiler

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "compiler/test-node.cc"
#include "compiler/test-operator.cc"
#include "compiler/test-representation-change.cc"
#include "compiler/test-run-calls-to-external-references.cc"
#include "compiler/test-run-load-store.cc"
#include "compiler/test-select-combine.cc"
#include "compiler/test-verify-type.cc"
