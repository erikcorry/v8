#!/usr/bin/env python3

# Copyright 2026 the V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Cluster build file utility for V8.

This script has two modes:

1. Compute filenames (--compute-filenames):
   Called by GN at gen-time to determine output filenames for cluster files.
   Outputs one cluster filename per line.

   Usage:
     cluster_files.py --compute-filenames --cluster-size <n> --prefix <prefix> \
         <file1> [file2] ...

2. Generate files (--generate):
   Called at build-time to create the actual cluster files with #include directives.

   Usage:
     cluster_files.py --generate --output-dir <dir> --prefix <prefix> \
         --cluster-size <n> [--strip-prefix <prefix>] [--include-prefix <prefix>] \
         <file1.cc> [file2.cc] ...
"""

import argparse
import os
import sys
from collections import defaultdict


def get_directory(filepath):
  """Get the directory part of a filepath.

  Files ending in -tsa.cc are grouped into a virtual 'tsa' subdirectory
  to keep them separate from other files. This avoids conflicts between
  TurboShaft Assembler files (which use turboshaft::Operation) and other
  files (which may use the Operation enum).
  """
  dir_path = os.path.dirname(filepath)
  basename = os.path.basename(filepath)
  if basename.endswith('-tsa.cc'):
    return os.path.join(dir_path, 'tsa')
  return dir_path


def sanitize_dir_name(dir_path):
  """Convert a directory path to a safe cluster name component."""
  # Replace path separators with hyphens
  name = dir_path.replace('\\', '/').replace('/', '-')
  # Remove any leading/trailing hyphens
  name = name.strip('-')
  # If empty, use 'root'
  return name if name else 'root'


def group_files_by_directory(files):
  """Group files by their directory."""
  files_by_dir = defaultdict(list)
  for f in files:
    dir_path = get_directory(f)
    files_by_dir[dir_path].append(f)
  return files_by_dir


def compute_cluster_info(files, cluster_size, prefix):
  """Compute cluster information (filenames and file assignments).

  Returns a list of tuples: (cluster_filename, list_of_source_files)
  """
  clusters = []

  # Group by directory
  files_by_dir = group_files_by_directory(files)
  for dir_path in sorted(files_by_dir.keys()):
    dir_files = sorted(files_by_dir[dir_path])
    dir_name = sanitize_dir_name(dir_path)
    cluster_num = 1
    for i in range(0, len(dir_files), cluster_size):
      chunk = dir_files[i:i + cluster_size]
      cluster_filename = f'{prefix}-{dir_name}-cluster-{cluster_num}.cc'
      clusters.append((cluster_filename, chunk))
      cluster_num += 1

  return clusters


def generate_cluster_content(files, cluster_name, prefix, strip_prefix,
                             include_prefix):
  """Generate the content of a cluster file."""
  lines = [
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


def cmd_compute_filenames(args):
  """Compute and print cluster filenames."""
  files = args.files

  if not files:
    return 0

  clusters = compute_cluster_info(files, args.cluster_size, args.prefix)

  for cluster_filename, _ in clusters:
    print(cluster_filename)

  return 0


def cmd_generate(args):
  """Generate cluster files."""
  files = args.files

  if not files:
    return 0

  # Create output directory if needed
  os.makedirs(args.output_dir, exist_ok=True)

  clusters = compute_cluster_info(files, args.cluster_size, args.prefix)

  for cluster_filename, source_files in clusters:
    cluster_name = cluster_filename[len(args.prefix) +
                                    1:-3]  # Remove prefix- and .cc
    cluster_path = os.path.join(args.output_dir, cluster_filename)

    content = generate_cluster_content(source_files, cluster_name, args.prefix,
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

  return 0


def main():
  parser = argparse.ArgumentParser(
      description='Cluster build file utility for V8.')

  # Mode selection (mutually exclusive)
  mode_group = parser.add_mutually_exclusive_group(required=True)
  mode_group.add_argument(
      '--compute-filenames',
      action='store_true',
      help='Compute and print cluster filenames (for GN gen-time)')
  mode_group.add_argument(
      '--generate',
      action='store_true',
      help='Generate cluster files (for build-time)')

  # Common arguments
  parser.add_argument(
      '--prefix', required=True, help='Prefix for cluster file names')
  parser.add_argument(
      '--cluster-size',
      type=int,
      default=10,
      help='Number of files per cluster (default: 10)')

  # Generate-specific arguments
  parser.add_argument(
      '--output-dir',
      help='Directory to write cluster files to (required for --generate)')
  parser.add_argument(
      '--strip-prefix',
      default='',
      help='Prefix to strip from file paths in #include directives')
  parser.add_argument(
      '--include-prefix',
      default='',
      help='Prefix to prepend to file paths in #include directives')

  parser.add_argument('files', nargs='*', help='Source files to cluster')

  args = parser.parse_args()

  # Validate arguments
  if args.generate and not args.output_dir:
    parser.error('--output-dir is required when using --generate')

  if not args.files:
    return 0

  if args.compute_filenames:
    return cmd_compute_filenames(args)
  else:
    return cmd_generate(args)


if __name__ == '__main__':
  sys.exit(main())
