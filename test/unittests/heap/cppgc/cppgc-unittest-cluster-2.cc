// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/unittests/heap/cppgc/gc-info-unittest.cc"
#include "test/unittests/heap/cppgc/gc-invoker-unittest.cc"
#include "test/unittests/heap/cppgc/heap-growing-unittest.cc"
#include "test/unittests/heap/cppgc/heap-object-header-unittest.cc"
#include "test/unittests/heap/cppgc/heap-page-unittest.cc"
#include "test/unittests/heap/cppgc/heap-registry-unittest.cc"
#include "test/unittests/heap/cppgc/heap-statistics-collector-unittest.cc"
#include "test/unittests/heap/cppgc/heap-unittest.cc"
#include "test/unittests/heap/cppgc/liveness-broker-unittest.cc"
#include "test/unittests/heap/cppgc/logging-unittest.cc"
#include "test/unittests/heap/cppgc/marker-unittest.cc"
#include "test/unittests/heap/cppgc/marking-verifier-unittest.cc"
