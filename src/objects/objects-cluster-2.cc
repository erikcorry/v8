// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/objects/fixed-array.cc"
#include "src/objects/instruction-stream.cc"
#include "src/objects/js-array-buffer.cc"
#include "src/objects/js-atomics-synchronization.cc"
#include "src/objects/js-disposable-stack.cc"
#include "src/objects/js-raw-json.cc"
#include "src/objects/js-regexp.cc"
#include "src/objects/js-segment-iterator.cc"
#include "src/objects/js-segmenter.cc"
#include "src/objects/js-segments.cc"
#include "src/objects/js-struct.cc"

#endif  // V8_CLUSTER_BUILD
