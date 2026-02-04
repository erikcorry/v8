// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD

#include "src/objects/js-temporal-helpers.cc"
#include "src/objects/js-weak-refs.cc"
#include "src/objects/managed.cc"
#include "src/objects/number-string-cache.cc"
#include "src/objects/object-type.cc"
#include "src/objects/option-utils.cc"
#include "src/objects/property-descriptor.cc"
#include "src/objects/property.cc"
#include "src/objects/regexp-match-info.cc"
#include "src/objects/simd.cc"
#include "src/objects/string-comparator.cc"

#endif  // V8_CLUSTER_BUILD
