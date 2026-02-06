// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/parsing
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/parsing/func-name-inferrer.cc"
#include "src/parsing/parse-info.cc"
#include "src/parsing/pending-compilation-error-handler.cc"
#include "src/parsing/rewriter.cc"
#include "src/parsing/scanner-character-streams.cc"
#include "src/parsing/scanner.cc"
