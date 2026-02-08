// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/heap

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "heap/test-alloc.cc"
#include "heap/test-array-buffer-sweeper.cc"
#include "heap/test-compaction.cc"
#include "heap/test-concurrent-allocation.cc"
#include "heap/test-concurrent-marking.cc"
#include "heap/test-external-string-tracker.cc"
#include "heap/test-incremental-marking.cc"
