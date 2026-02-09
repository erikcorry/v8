// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "heap/cppgc/allocation-unittest.cc"
#include "heap/cppgc/caged-heap-unittest.cc"
#include "heap/cppgc/compactor-unittest.cc"
#include "heap/cppgc/concurrent-marking-unittest.cc"
#include "heap/cppgc/concurrent-sweeper-unittest.cc"
#include "heap/cppgc/cross-thread-persistent-unittest.cc"
#include "heap/cppgc/custom-spaces-unittest.cc"
#include "heap/cppgc/ephemeron-pair-unittest.cc"
#include "heap/cppgc/explicit-management-unittest.cc"
#include "heap/cppgc/finalizer-trait-unittest.cc"
#include "heap/cppgc/free-list-unittest.cc"
#include "heap/cppgc/garbage-collected-unittest.cc"
