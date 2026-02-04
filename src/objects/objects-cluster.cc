// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/objects/abstract-code.cc"            // nogncheck
#include "src/objects/bigint.cc"                   // nogncheck
#include "src/objects/bytecode-array.cc"           // nogncheck
#include "src/objects/code.cc"                     // nogncheck
#include "src/objects/compilation-cache-table.cc"  // nogncheck
#include "src/objects/debug-objects.cc"            // nogncheck
#include "src/objects/deoptimization-data.cc"      // nogncheck
#include "src/objects/dependent-code.cc"           // nogncheck
#include "src/objects/elements-kind.cc"            // nogncheck
#include "src/objects/embedder-data-array.cc"      // nogncheck
#include "src/objects/field-type.cc"               // nogncheck

#endif  // V8_CLUSTER_BUILD
