#!/bin/bash

set -euo pipefail
shopt -s nullglob

patterns=(
  "*.png"
  "*.dat"
  "*.bin"
  "*_q*.wav"
)

for pattern in "${patterns[@]}"; do
    files=( $pattern )
    if [ ${#files[@]} -gt 0 ]; then
        echo "Removing ${#files[@]} file(s) matching: $pattern"
        rm -f "${files[@]}"
    else
        echo "No files found for: $pattern"
    fi
done

echo "Cleanup complete."
