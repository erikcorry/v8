// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/runtime
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/runtime/runtime-numbers.cc"
#include "src/runtime/runtime-operators.cc"
#include "src/runtime/runtime-promise.cc"
#include "src/runtime/runtime-proxy.cc"
#include "src/runtime/runtime-shadow-realm.cc"
#include "src/runtime/runtime-strings.cc"
#include "src/runtime/runtime-symbol.cc"
#include "src/runtime/runtime-trace.cc"
#include "src/runtime/runtime-weak-refs.cc"
#include "src/runtime/runtime.cc"
