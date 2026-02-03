// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/compiler/turboshaft
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/compiler/turboshaft/debug-feature-lowering-phase.cc"  // nogncheck
#include "src/compiler/turboshaft/decompression-optimization-phase.cc"  // nogncheck
#include "src/compiler/turboshaft/graph-visualizer.cc"  // nogncheck
#include "src/compiler/turboshaft/graph.cc"             // nogncheck
#include "src/compiler/turboshaft/loop-finder.cc"       // nogncheck
#include "src/compiler/turboshaft/phase.cc"             // nogncheck
#include "src/compiler/turboshaft/sidetable.cc"         // nogncheck
#include "src/compiler/turboshaft/typer.cc"             // nogncheck
#include "src/compiler/turboshaft/use-map.cc"           // nogncheck
#include "src/compiler/turboshaft/wasm-gc-typed-optimization-reducer.cc"  // nogncheck
#include "src/compiler/turboshaft/wasm-shuffle-reducer.cc"  // nogncheck

#endif  // V8_CLUSTER_BUILD
