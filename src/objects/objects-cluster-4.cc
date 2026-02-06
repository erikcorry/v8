// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/objects/string-forwarding-table.cc"
#include "src/objects/swiss-name-dictionary.cc"
#include "src/objects/symbol-table.cc"
#include "src/objects/synthetic-module.cc"
#include "src/objects/tagged-impl.cc"
#include "src/objects/template-objects.cc"
#include "src/objects/transitions.cc"
#include "src/objects/visitors.cc"
#include "src/objects/waiter-queue-node.cc"
