// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "heap/cppgc/marking-visitor-unittest.cc"
#include "heap/cppgc/member-unittest.cc"
#include "heap/cppgc/metric-recorder-unittest.cc"
#include "heap/cppgc/minor-gc-unittest.cc"
#include "heap/cppgc/name-trait-unittest.cc"
#include "heap/cppgc/object-size-trait-unittest.cc"
#include "heap/cppgc/object-start-bitmap-unittest.cc"
#include "heap/cppgc/page-memory-unittest.cc"
#include "heap/cppgc/persistent-family-unittest.cc"
#include "heap/cppgc/platform-unittest.cc"
#include "heap/cppgc/prefinalizer-unittest.cc"
#include "heap/cppgc/sanitizer-unittest.cc"
