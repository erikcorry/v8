// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/compiler/turboshaft

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/compiler/turboshaft/call-runtime-unittest.cc"
#include "test/unittests/compiler/turboshaft/control-flow-unittest.cc"
#include "test/unittests/compiler/turboshaft/load-store-address-hoisting-unittest.cc"
#include "test/unittests/compiler/turboshaft/opmask-unittest.cc"
#include "test/unittests/compiler/turboshaft/reducer-test.cc"
#include "test/unittests/compiler/turboshaft/snapshot-table-unittest.cc"
#include "test/unittests/compiler/turboshaft/store-store-elimination-reducer-unittest.cc"
#include "test/unittests/compiler/turboshaft/turboshaft-typer-unittest.cc"
#include "test/unittests/compiler/turboshaft/turboshaft-types-unittest.cc"
#include "test/unittests/compiler/turboshaft/typeswitch-unittest.cc"
