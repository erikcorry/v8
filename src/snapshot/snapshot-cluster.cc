// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/snapshot
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/snapshot/code-serializer.cc"
#include "src/snapshot/context-deserializer.cc"
#include "src/snapshot/object-deserializer.cc"
#include "src/snapshot/read-only-deserializer.cc"
#include "src/snapshot/read-only-serializer.cc"
#include "src/snapshot/serializer-deserializer.cc"
#include "src/snapshot/shared-heap-deserializer.cc"
#include "src/snapshot/shared-heap-serializer.cc"
#include "src/snapshot/snapshot-source-sink.cc"
#include "src/snapshot/startup-deserializer.cc"
#include "src/snapshot/startup-serializer.cc"

#endif  // V8_CLUSTER_BUILD
