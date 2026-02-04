#!/usr/bin/env python3
# Copyright 2026 The V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Extracts source files included by cluster build files.

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


def extract_includes(cluster_file):
  """Extracts .cc file paths from #include directives in a cluster file."""
  includes = []
  with open(cluster_file, 'r') as f:
    for line in f:
      match = INCLUDE_RE.match(line)
      if match:
        includes.append(match.group(1))
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
