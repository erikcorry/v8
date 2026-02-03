// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/runtime
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/runtime/runtime-numbers.cc"       // nogncheck
#include "src/runtime/runtime-operators.cc"     // nogncheck
#include "src/runtime/runtime-promise.cc"       // nogncheck
#include "src/runtime/runtime-proxy.cc"         // nogncheck
#include "src/runtime/runtime-shadow-realm.cc"  // nogncheck
#include "src/runtime/runtime-strings.cc"       // nogncheck
#include "src/runtime/runtime-symbol.cc"        // nogncheck
#include "src/runtime/runtime-trace.cc"         // nogncheck
#include "src/runtime/runtime-weak-refs.cc"     // nogncheck
#include "src/runtime/runtime.cc"               // nogncheck

#endif  // V8_CLUSTER_BUILD
