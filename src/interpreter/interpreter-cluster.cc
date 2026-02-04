// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/interpreter
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/interpreter/bytecode-array-iterator.cc"         // nogncheck
#include "src/interpreter/bytecode-array-random-iterator.cc"  // nogncheck
#include "src/interpreter/bytecode-array-writer.cc"           // nogncheck
#include "src/interpreter/bytecode-decoder.cc"                // nogncheck
#include "src/interpreter/bytecode-flags-and-tokens.cc"       // nogncheck
#include "src/interpreter/bytecode-label.cc"                  // nogncheck
#include "src/interpreter/constant-array-builder.cc"          // nogncheck
#include "src/interpreter/control-flow-builders.cc"           // nogncheck
#include "src/interpreter/handler-table-builder.cc"           // nogncheck
#include "src/interpreter/prototype-assignment-sequence-builder.cc"  // nogncheck

#endif  // V8_CLUSTER_BUILD
