// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/objects
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#ifdef V8_CLUSTER_BUILD
#undef V8_CLUSTER_BUILD

#include "src/objects/js-temporal-helpers.cc"  // nogncheck
#include "src/objects/js-weak-refs.cc"         // nogncheck
#include "src/objects/managed.cc"              // nogncheck
#include "src/objects/number-string-cache.cc"  // nogncheck
#include "src/objects/object-type.cc"          // nogncheck
#include "src/objects/option-utils.cc"         // nogncheck
#include "src/objects/property-descriptor.cc"  // nogncheck
#include "src/objects/property.cc"             // nogncheck
#include "src/objects/regexp-match-info.cc"    // nogncheck
#include "src/objects/simd.cc"                 // nogncheck
#include "src/objects/string-comparator.cc"    // nogncheck

#endif  // V8_CLUSTER_BUILD
