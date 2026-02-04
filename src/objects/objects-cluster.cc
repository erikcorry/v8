// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/objects/abstract-code.cc"
#include "src/objects/bigint.cc"
#include "src/objects/bytecode-array.cc"
#include "src/objects/code.cc"
#include "src/objects/compilation-cache-table.cc"
#include "src/objects/debug-objects.cc"
#include "src/objects/deoptimization-data.cc"
#include "src/objects/dependent-code.cc"
#include "src/objects/elements-kind.cc"
#include "src/objects/embedder-data-array.cc"
#include "src/objects/field-type.cc"

#endif  // V8_CLUSTER_BUILD
