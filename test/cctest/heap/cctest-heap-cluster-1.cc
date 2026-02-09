// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for test/cctest/heap

#pragma clang diagnostic ignored "-Wheader-hygiene"

#include "test/cctest/heap/test-alloc.cc"
#include "test/cctest/heap/test-array-buffer-sweeper.cc"
#include "test/cctest/heap/test-compaction.cc"
#include "test/cctest/heap/test-concurrent-allocation.cc"
#include "test/cctest/heap/test-concurrent-marking.cc"
#include "test/cctest/heap/test-external-string-tracker.cc"
#include "test/cctest/heap/test-incremental-marking.cc"
