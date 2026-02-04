// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/compiler/turboshaft
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/compiler/turboshaft/debug-feature-lowering-phase.cc"
#include "src/compiler/turboshaft/decompression-optimization-phase.cc"
#include "src/compiler/turboshaft/graph-visualizer.cc"
#include "src/compiler/turboshaft/graph.cc"
#include "src/compiler/turboshaft/loop-finder.cc"
#include "src/compiler/turboshaft/phase.cc"
#include "src/compiler/turboshaft/sidetable.cc"
#include "src/compiler/turboshaft/typer.cc"
#include "src/compiler/turboshaft/use-map.cc"
#include "src/compiler/turboshaft/wasm-gc-typed-optimization-reducer.cc"
#include "src/compiler/turboshaft/wasm-shuffle-reducer.cc"

#endif  // V8_CLUSTER_BUILD
