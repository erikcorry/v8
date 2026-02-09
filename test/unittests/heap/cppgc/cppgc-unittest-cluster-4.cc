// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/heap/cppgc/source-location-unittest.cc"
#include "test/unittests/heap/cppgc/stack-unittest.cc"
#include "test/unittests/heap/cppgc/stats-collector-scopes-unittest.cc"
#include "test/unittests/heap/cppgc/stats-collector-unittest.cc"
#include "test/unittests/heap/cppgc/sweeper-unittest.cc"
#include "test/unittests/heap/cppgc/testing-unittest.cc"
#include "test/unittests/heap/cppgc/visitor-unittest.cc"
#include "test/unittests/heap/cppgc/weak-container-unittest.cc"
#include "test/unittests/heap/cppgc/workloads-unittest.cc"
#include "test/unittests/heap/cppgc/write-barrier-unittest.cc"
