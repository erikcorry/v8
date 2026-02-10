#!/usr/bin/env python3

# Copyright 2026 the V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Computes cluster file names for a given set of source files.

This script is called by GN at gen-time to determine the output filenames
for cluster files that will be generated during the build.

Usage:
  compute_cluster_filenames.py <cluster_size> <prefix> [--group-by-dir] [--skip-arch-dirs] <file1> [file2] ...

Output:
  One cluster filename per line (just the filename, no directory):
    prefix-cluster-1.cc
    prefix-cluster-2.cc
    ...
  Or with --group-by-dir:
    prefix-dirname-cluster-1.cc
    prefix-dirname-cluster-2.cc
    ...
"""

import math
import os
import sys
from collections import defaultdict


# Architecture-specific directory names to skip
ARCH_DIRS = frozenset([
    'x64', 'arm64', 'arm', 'ia32', 'mips64', 'mips64el',
    'riscv64', 'riscv32', 'loong64', 'ppc64', 's390x', 's390'
])


def is_arch_specific(filepath):
  """Check if a file is in an architecture-specific directory."""
  parts = filepath.replace('\\', '/').split('/')
  for part in parts:
    if part in ARCH_DIRS:
      return True
  return False


def get_directory(filepath):
  """Get the directory part of a filepath."""
  return os.path.dirname(filepath)


def sanitize_dir_name(dir_path):
  """Convert a directory path to a safe cluster name component."""
  # Replace path separators with hyphens
  name = dir_path.replace('\\', '/').replace('/', '-')
  # Remove any leading/trailing hyphens
  name = name.strip('-')
  # If empty, use 'root'
  return name if name else 'root'


def main():
  if len(sys.argv) < 3:
    print(f'Usage: {sys.argv[0]} <cluster_size> <prefix> [--group-by-dir] [--skip-arch-dirs] <files...>',
          file=sys.stderr)
    return 1

  cluster_size = int(sys.argv[1])
  prefix = sys.argv[2]

  # Parse flags and files
  group_by_dir = False
  skip_arch_dirs = False
  files = []

  for arg in sys.argv[3:]:
    if arg == '--group-by-dir':
      group_by_dir = True
    elif arg == '--skip-arch-dirs':
      skip_arch_dirs = True
    else:
      files.append(arg)

  if not files:
    return 0

  # Filter out arch-specific files if requested
  if skip_arch_dirs:
    files = [f for f in files if not is_arch_specific(f)]

  if not files:
    return 0

  # Generate cluster filenames
  if group_by_dir:
    # Group files by directory
    files_by_dir = defaultdict(list)
    for f in files:
      dir_path = get_directory(f)
      files_by_dir[dir_path].append(f)

    # Generate cluster filenames for each directory
    for dir_path in sorted(files_by_dir.keys()):
      dir_files = files_by_dir[dir_path]
      dir_name = sanitize_dir_name(dir_path)
      num_clusters = math.ceil(len(dir_files) / cluster_size)
      for i in range(1, num_clusters + 1):
        print(f'{prefix}-{dir_name}-cluster-{i}.cc')
  else:
    num_clusters = math.ceil(len(files) / cluster_size)
    for i in range(1, num_clusters + 1):
      print(f'{prefix}-cluster-{i}.cc')

  return 0


if __name__ == '__main__':
  sys.exit(main())
