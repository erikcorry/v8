// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/heap/cppgc

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "heap/cppgc/gc-info-unittest.cc"
#include "heap/cppgc/gc-invoker-unittest.cc"
#include "heap/cppgc/heap-growing-unittest.cc"
#include "heap/cppgc/heap-object-header-unittest.cc"
#include "heap/cppgc/heap-page-unittest.cc"
#include "heap/cppgc/heap-registry-unittest.cc"
#include "heap/cppgc/heap-statistics-collector-unittest.cc"
#include "heap/cppgc/heap-unittest.cc"
#include "heap/cppgc/liveness-broker-unittest.cc"
#include "heap/cppgc/logging-unittest.cc"
#include "heap/cppgc/marker-unittest.cc"
#include "heap/cppgc/marking-verifier-unittest.cc"
