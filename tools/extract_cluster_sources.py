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
"""

import re
import sys

# Matches: #include "src/foo/bar.cc"
INCLUDE_RE = re.compile(r'^\s*#include\s+"(src/[^"]+\.cc)"')
# Matches blank lines, comments (// or /* style), and copyright/license headers
BLANK_RE = re.compile(r'^\s*$')
COMMENT_RE = re.compile(r'^\s*(//|/\*|\*|\*/)')


def extract_includes(cluster_file):
  """Extracts .cc file paths from #include directives in a cluster file.

  Also validates that the file only contains blank lines, comments, and
  valid include directives.
  """
  includes = []
  with open(cluster_file, 'r') as f:
    for line_num, line in enumerate(f, 1):
      include_match = INCLUDE_RE.match(line)
      if include_match:
        includes.append(include_match.group(1))
      elif not BLANK_RE.match(line) and not COMMENT_RE.match(line):
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
