#!/usr/bin/env python3

# Copyright 2026 the V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Generates cluster build files from a list of source files.

This script takes a list of .cc files and generates cluster files that
#include groups of ~10 files each. This is used for torque-generated files
where the cluster files need to be generated during the build.

Usage:
  generate_cluster_files.py --output-dir <dir> --prefix <prefix> \
      --cluster-size <n> --strip-prefix <prefix> <file1.cc> [file2.cc] ...

Output:
  Creates cluster files named <prefix>-cluster-<n>.cc in <output-dir>.
  Or with --group-by-dir: <prefix>-<dirname>-cluster-<n>.cc
"""

import argparse
import os
import sys
from collections import defaultdict

# Architecture-specific directory names to skip
ARCH_DIRS = frozenset([
    'x64', 'arm64', 'arm', 'ia32', 'mips64', 'mips64el', 'riscv64', 'riscv32',
    'loong64', 'ppc64', 's390x', 's390'
])


def generate_cluster_content(files, cluster_name, prefix, strip_prefix,
                             include_prefix):
  """Generate the content of a cluster file."""
  lines = [
      '// Copyright 2026 the V8 project authors. All rights reserved.',
      '// Use of this source code is governed by a BSD-style license that can be',
      '// found in the LICENSE file.',
      '',
      f'// Auto-generated cluster build file for {prefix} ({cluster_name})',
      '',
      '#pragma clang diagnostic ignored "-Wheader-hygiene"',
      '',
  ]
  for f in files:
    # Strip the prefix to get a path relative to the cluster file location
    if strip_prefix and f.startswith(strip_prefix):
      include_path = f[len(strip_prefix):]
    else:
      include_path = f
    # Prepend include_prefix if specified
    if include_prefix:
      include_path = include_prefix + include_path
    lines.append(f'#include "{include_path}"')
  lines.append('')
  return '\n'.join(lines)


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
  parser = argparse.ArgumentParser(
      description='Generate cluster build files from source files.')
  parser.add_argument(
      '--output-dir', required=True, help='Directory to write cluster files to')
  parser.add_argument(
      '--prefix', required=True, help='Prefix for cluster file names')
  parser.add_argument(
      '--cluster-size',
      type=int,
      default=10,
      help='Number of files per cluster (default: 10)')
  parser.add_argument(
      '--strip-prefix',
      default='',
      help='Prefix to strip from file paths in #include directives')
  parser.add_argument(
      '--include-prefix',
      default='',
      help='Prefix to prepend to file paths in #include directives')
  parser.add_argument(
      '--group-by-dir',
      action='store_true',
      help='Group files by directory before clustering')
  parser.add_argument(
      '--skip-arch-dirs',
      action='store_true',
      help='Skip files in architecture-specific directories')
  parser.add_argument(
      '--exclude',
      action='append',
      default=[],
      help='File patterns to exclude from clustering (can be repeated)')
  parser.add_argument('files', nargs='*', help='Source files to cluster')

  args = parser.parse_args()

  if not args.files:
    return 0

  # Filter out arch-specific files if requested
  files = args.files
  if args.skip_arch_dirs:
    files = [f for f in files if not is_arch_specific(f)]

  if not files:
    return 0

  # Create output directory if needed
  os.makedirs(args.output_dir, exist_ok=True)

  cluster_files = []

  if args.group_by_dir:
    # Group files by directory
    files_by_dir = defaultdict(list)
    for f in files:
      dir_path = get_directory(f)
      files_by_dir[dir_path].append(f)

    # Process each directory separately with its own sequence
    for dir_path in sorted(files_by_dir.keys()):
      dir_files = sorted(files_by_dir[dir_path])
      dir_name = sanitize_dir_name(dir_path)
      cluster_num = 1
      for i in range(0, len(dir_files), args.cluster_size):
        chunk = dir_files[i:i + args.cluster_size]
        cluster_name = f'{dir_name}-cluster-{cluster_num}'
        cluster_filename = f'{args.prefix}-{cluster_name}.cc'
        cluster_path = os.path.join(args.output_dir, cluster_filename)

        content = generate_cluster_content(chunk, cluster_name, args.prefix,
                                           args.strip_prefix,
                                           args.include_prefix)

        # Only write if content changed (to avoid unnecessary rebuilds)
        write_needed = True
        if os.path.exists(cluster_path):
          with open(cluster_path, 'r') as f:
            if f.read() == content:
              write_needed = False

        if write_needed:
          with open(cluster_path, 'w') as f:
            f.write(content)

        cluster_files.append(cluster_path)
        cluster_num += 1
  else:
    # Original behavior: cluster all files together
    files = sorted(files)
    cluster_num = 1
    for i in range(0, len(files), args.cluster_size):
      chunk = files[i:i + args.cluster_size]
      cluster_name = f'cluster-{cluster_num}'
      cluster_filename = f'{args.prefix}-{cluster_name}.cc'
      cluster_path = os.path.join(args.output_dir, cluster_filename)

      content = generate_cluster_content(chunk, cluster_name, args.prefix,
                                         args.strip_prefix, args.include_prefix)

      # Only write if content changed (to avoid unnecessary rebuilds)
      write_needed = True
      if os.path.exists(cluster_path):
        with open(cluster_path, 'r') as f:
          if f.read() == content:
            write_needed = False

      if write_needed:
        with open(cluster_path, 'w') as f:
          f.write(content)

      cluster_files.append(cluster_path)
      cluster_num += 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
