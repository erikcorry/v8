// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/unittests/libplatform

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "libplatform/default-job-unittest.cc"
#include "libplatform/default-platform-unittest.cc"
#include "libplatform/default-worker-threads-task-runner-unittest.cc"
#include "libplatform/single-threaded-default-platform-unittest.cc"
#include "libplatform/task-queue-unittest.cc"
#include "libplatform/tracing-unittest.cc"
#include "libplatform/worker-thread-unittest.cc"
