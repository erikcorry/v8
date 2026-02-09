#!/usr/bin/env python3

# Copyright 2026 the V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
"""
Extracts source files included by cluster build files.

This script reads cluster build files (*-cluster-*.cc) and extracts the paths
of .cc files they include. The output is used by GN to exclude these files
from the build when cluster build is enabled.

Usage:
  extract_cluster_sources.py <cluster_file1> [cluster_file2] ...

Output:
  One source file path per line, suitable for GN's "list lines" format.
  Paths are relative to the same base directory as the cluster file paths.

Note:
  Include paths in cluster files are resolved relative to the cluster file's
  directory. For example, if cluster file "heap/cppgc/cluster.cc" includes
  "foo.cc", the output will be "heap/cppgc/foo.cc".
"""

import os
import re
import sys

# Matches: #include "src/foo/bar.cc" or #include "foo.cc" (relative)
INCLUDE_RE = re.compile(r'^\s*#include\s+"([^"]+\.cc)"')
# Matches blank lines, comments (// or /* style), and copyright/license headers
BLANK_RE = re.compile(r'^\s*$')
COMMENT_RE = re.compile(r'^\s*(//|/\*|\*|\*/)')
# Matches #pragma, namespace declarations, and } lines (for cluster file structure)
PRAGMA_RE = re.compile(r'^\s*#pragma\s+')
NAMESPACE_RE = re.compile(r'^\s*(namespace\s+\w*\s*\{|\})')


def extract_includes(cluster_file):
  """Extracts .cc file paths from #include directives in a cluster file.

  Include paths in cluster files can be:
  1. Relative to BUILD.gn (e.g., "foo.cc" or "subdir/foo.cc")
  2. Absolute from repo root (e.g., "test/unittests/heap/cppgc/foo.cc")

  For absolute paths, this function strips the BUILD.gn directory prefix
  to produce paths relative to BUILD.gn that match the sources list.

  Also validates that the file only contains blank lines, comments, and
  valid include directives.
  """
  # Known BUILD.gn directory prefixes that should be stripped from include paths
  # These are directories where BUILD.gn lives, so repo-root paths need the
  # prefix stripped to match the BUILD.gn-relative source paths.
  # Note: src/ is NOT included because the root BUILD.gn uses src/-prefixed paths.
  KNOWN_BUILDGN_DIRS = ['test/unittests/', 'test/cctest/']

  includes = []
  with open(cluster_file, 'r') as f:
    for line_num, line in enumerate(f, 1):
      include_match = INCLUDE_RE.match(line)
      if include_match:
        include_path = include_match.group(1)
        # Strip known BUILD.gn directory prefix if present
        for prefix in KNOWN_BUILDGN_DIRS:
          if include_path.startswith(prefix):
            include_path = include_path[len(prefix):]
            break
        includes.append(include_path)
      elif (not BLANK_RE.match(line) and not COMMENT_RE.match(line) and
            not PRAGMA_RE.match(line) and not NAMESPACE_RE.match(line)):
        print(
            f'Error: {cluster_file}:{line_num}: invalid line: {line.rstrip()}',
            file=sys.stderr)
        sys.exit(1)
  return includes


def main():
  if len(sys.argv) < 2:
    return 0

  all_includes = []
  for cluster_file in sys.argv[1:]:
    all_includes.extend(extract_includes(cluster_file))

  for path in all_includes:
    print(path)

  return 0


if __name__ == '__main__':
  sys.exit(main())
