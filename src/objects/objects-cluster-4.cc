// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that compile in similar time
// to improve build parallelism.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/objects/string-forwarding-table.cc"  // nogncheck
#include "src/objects/swiss-name-dictionary.cc"    // nogncheck
#include "src/objects/symbol-table.cc"             // nogncheck
#include "src/objects/synthetic-module.cc"         // nogncheck
#include "src/objects/tagged-impl.cc"              // nogncheck
#include "src/objects/template-objects.cc"         // nogncheck
#include "src/objects/transitions.cc"              // nogncheck
#include "src/objects/visitors.cc"                 // nogncheck
#include "src/objects/waiter-queue-node.cc"        // nogncheck

#endif  // V8_CLUSTER_BUILD
