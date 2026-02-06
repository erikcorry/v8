// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cluster build file for src/ast
// This file includes multiple .cc files that include roughly the same .h files
// to reduce redundant processing of large include sequences.

#include "src/ast/ast-value-factory.cc"
#include "src/ast/ast.cc"
#include "src/ast/modules.cc"
#include "src/ast/prettyprinter.cc"
