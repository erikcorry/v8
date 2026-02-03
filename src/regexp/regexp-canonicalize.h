// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_REGEXP_REGEXP_CANONICALIZE_H_
#define V8_REGEXP_REGEXP_CANONICALIZE_H_

// WARNING: This header is used by gen-regexp-special-case.cc which is a
// build-time tool that should not depend on V8 internals. Do not include
// any V8 headers here (src/base/*, src/common/*, etc.).

#include <cassert>

#include "unicode/uchar.h"
#include "unicode/unistr.h"

// This implements ECMAScript 2020 21.2.2.8.2 (Runtime Semantics:
// Canonicalize) step 3, which is used to determine whether
// characters match when ignoreCase is true and unicode is false.
inline UChar32 RegExpCanonicalize(UChar32 ch) {
  // a. Assert: ch is a UTF-16 code unit.
  assert(ch <= 0xffff);

  // b. Let s be the String value consisting of the single code unit ch.
  icu::UnicodeString s(ch);

  // c. Let u be the same result produced as if by performing the algorithm
  // for String.prototype.toUpperCase using s as the this value.
  // d. Assert: Type(u) is String.
  icu::UnicodeString& u = s.toUpper();

  // e. If u does not consist of a single code unit, return ch.
  if (u.length() != 1) {
    return ch;
  }

  // f. Let cu be u's single code unit element.
  UChar32 cu = u.char32At(0);

  // g. If the value of ch >= 128 and the value of cu < 128, return ch.
  if (ch >= 128 && cu < 128) {
    return ch;
  }

  // h. Return cu.
  return cu;
}

#endif  // V8_REGEXP_REGEXP_CANONICALIZE_H_
