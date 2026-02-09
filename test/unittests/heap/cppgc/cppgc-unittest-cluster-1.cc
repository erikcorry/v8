// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/heap/cppgc/allocation-unittest.cc"
#include "test/unittests/heap/cppgc/caged-heap-unittest.cc"
#include "test/unittests/heap/cppgc/compactor-unittest.cc"
#include "test/unittests/heap/cppgc/concurrent-marking-unittest.cc"
#include "test/unittests/heap/cppgc/concurrent-sweeper-unittest.cc"
#include "test/unittests/heap/cppgc/cross-thread-persistent-unittest.cc"
#include "test/unittests/heap/cppgc/custom-spaces-unittest.cc"
#include "test/unittests/heap/cppgc/ephemeron-pair-unittest.cc"
#include "test/unittests/heap/cppgc/explicit-management-unittest.cc"
#include "test/unittests/heap/cppgc/finalizer-trait-unittest.cc"
#include "test/unittests/heap/cppgc/free-list-unittest.cc"
#include "test/unittests/heap/cppgc/garbage-collected-unittest.cc"
