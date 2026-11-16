#!/usr/bin/env bash
# build a tunable binary (env overrides) inside HackerRank's main: tb.sh src out
g++ -std=c++23 -O2 -Wno-deprecated-declarations -DP2TUNE -DHR_SOLUTION="\"$(realpath "$1")\"" -o "$2" /home/wangz/c3/p2/hr_main.cpp
