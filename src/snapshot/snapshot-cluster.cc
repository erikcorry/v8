// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/snapshot
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/snapshot/code-serializer.cc"           // nogncheck
#include "src/snapshot/context-deserializer.cc"      // nogncheck
#include "src/snapshot/object-deserializer.cc"       // nogncheck
#include "src/snapshot/read-only-deserializer.cc"    // nogncheck
#include "src/snapshot/read-only-serializer.cc"      // nogncheck
#include "src/snapshot/serializer-deserializer.cc"   // nogncheck
#include "src/snapshot/shared-heap-deserializer.cc"  // nogncheck
#include "src/snapshot/shared-heap-serializer.cc"    // nogncheck
#include "src/snapshot/snapshot-source-sink.cc"      // nogncheck
#include "src/snapshot/startup-deserializer.cc"      // nogncheck
#include "src/snapshot/startup-serializer.cc"        // nogncheck

#endif  // V8_CLUSTER_BUILD
