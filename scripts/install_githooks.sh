#!/bin/sh
set -eu

force=0
if [ "${1:-}" = "--force" ]; then
  force=1
fi

if [ ! -d ".git" ]; then
  echo "run from repo root (missing .git)" >&2
  exit 1
fi

src_dir="$(cd "$(dirname "$0")" && pwd)/githooks"
dst_dir=".git/hooks"

mkdir -p "$dst_dir"

for h in pre-commit commit-msg; do
  src="$src_dir/$h"
  dst="$dst_dir/$h"
  if [ ! -f "$src" ]; then
    echo "missing hook template: $src" >&2
    exit 1
  fi
  if [ -e "$dst" ] && [ "$force" -ne 1 ]; then
    echo "hook already exists: $dst (use --force to overwrite)" >&2
    exit 1
  fi
  cp -f "$src" "$dst"
  chmod +x "$dst" || true
done

echo "installed hooks to $dst_dir"

