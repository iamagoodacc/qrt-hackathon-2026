#!/usr/bin/env bash
# Build a solution EXACTLY the way HackerRank will: its real hidden main() (pN/hr_main.cpp) with
# your file pasted in where the stub function goes. If this builds and scores, the same file
# pasted into HackerRank will behave the same.
#
# Usage:   ./hrbuild.sh p1 p1/b/solve.cpp p1/b/solve
# Then:    python3 harness.py run p1 "./p1/b/solve" --save p1/b/solve.cpp --note "..." --time-limit 2
set -euo pipefail
P="$1"; SRC="$2"; OUT="$3"
ROOT="$(cd "$(dirname "$0")" && pwd)"
g++ -std=c++23 -O2 -Wno-deprecated-declarations \
    -DHR_SOLUTION="\"$(realpath "$SRC")\"" -o "$OUT" "$ROOT/$P/hr_main.cpp"
