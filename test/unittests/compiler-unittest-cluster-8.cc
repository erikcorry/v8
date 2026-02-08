// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/compiler/turboshaft

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "compiler/turboshaft/call-runtime-unittest.cc"
#include "compiler/turboshaft/control-flow-unittest.cc"
#include "compiler/turboshaft/late-load-elimination-reducer-unittest.cc"
#include "compiler/turboshaft/load-store-address-hoisting-unittest.cc"
#include "compiler/turboshaft/loop-unrolling-analyzer-unittest.cc"
#include "compiler/turboshaft/opmask-unittest.cc"
#include "compiler/turboshaft/reducer-test.cc"
#include "compiler/turboshaft/snapshot-table-unittest.cc"
#include "compiler/turboshaft/store-store-elimination-reducer-unittest.cc"
#include "compiler/turboshaft/turboshaft-typer-unittest.cc"
#include "compiler/turboshaft/turboshaft-types-unittest.cc"
#include "compiler/turboshaft/typeswitch-unittest.cc"
