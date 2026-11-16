# p2-b notes (main optimiser)

## Rules
- Input: R required words, F forbidden words (F may be 0). Output an N x N grid of A-Z.
- Grid is a torus; words read in straight lines in 8 directions (fwd/back, diag), wrapping.
- Every required word must appear somewhere; no forbidden word may appear anywhere (any dir, wrapping).
- N >= longest required word, else invalid (score 0). Forbidden word longer than N: treat as
  appearing if it occurs wrapping onto itself (stricter = safer).
- C++ stub: vector<vector<char>> generate_grid(vector<string> req, vector<string> forb); HR main prints
  rows with single spaces between letters.

## Scoring
S = total letters of required words (duplicates/substrings count!), density = S/N^2,
score = 100 * min(1, density/1.7). Only N matters. Full score once N <= floor(sqrt(S/1.7)).

## Limits
Constraints and time limit NOT given. Assume 2 s, budget ~1.5 s. Handle any size.

## Plan
- Drop required words that are substrings of another (or its reverse): free letters.
- For N from a feasible greedy upper bound downwards: SA over word placements (r, c, dir),
  cost = sum over cells of (#words covering - max same-letter count). Moves: random placement or
  anchor move (align one letter of the word on a grid cell with that letter).
- Free cells: a letter that occurs in no forbidden word (then free cells never complete one).
  Validate forbidden words on the final grid with a trie; reject + continue on violation.

## Log
- NEW BEST score=0.5131 (best 0.0820) max_time=1.50s idea=SA over placements, conflict cost, crossing moves, shrink N, filler letter absent from forbidden
- score=0.5188 (best 0.5252) max_time=1.50s idea=solve3: SA + occupancy reward 0.2, T 0.3->0.1, conflict-focused pick. ENGLISH set (p2/c/tests_eng) 0.728 vs c solve2 0.7035
- solve3.cpp now: exhaustive greedy init (all N*N*8 placements, min conflicts + 0.2*new cells), greedy-only
  scan upward from max(maxLen, fullN), then SA at N-1 from greedy init (no warm start).
  English set p2/b/tests_eng2 (40 tests, up to 150 words): 0.745 vs p2-c best 0.767 (c wins on big tests).
  Small tests: SA often beats c by one N. Next idea (not applied): min-conflicts move = reinsert a
  conflicting word at its best placement over the whole grid (P_BEST ~0.05-0.3).
- REASSIGNED to p4-b at ~19:25.
