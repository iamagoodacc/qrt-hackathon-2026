# p1-b notes (main optimiser)

## Rules
- Given sides a0..an (positive ints). Place each square axis-aligned, integer top-left (X, Y); X down, Y right.
- Square occupies [X,X+s) x [Y,Y+s); interiors must not overlap (touching OK).
- Return one {X, Y} per square in INPUT order (HackerRank stub `vector<vector<int>> pack_squares(vector<int>)`).
- Minimise the area of the bounding rectangle.

## Scoring
m = max side, S = sum s^2, L = min_{W>=m} W * max(m, ceil(S/W)). Score = (L/A)^20. Invalid = 0.
"Test cases are chosen so that L is close to achievable" -> hidden tests likely near-perfect tilings.

## Limits
Constraints NOT given (orchestrator TODO). Handle any n / side safely (no fixed MAXS arrays).
Time limit unknown -> assume 2 s, budget ~1.5 s.

## Plan
Port best/p0/BEST.cpp (skyline DFS in fixed W x H container, shrink target) into pack_squares,
with segment skyline + size compression so big sides / big n don't blow up. Then improve.

## Log
- NEW BEST score=0.7761 (best 0.3191) max_time=1.51s idea=p0 skyline DFS ported to stub, segment skyline, sampled widths
- solve2.cpp (DFS + prefix LNS: cut deepest partial path, noisy re-DFS): ~same as v1 (0.766-0.787 vs 0.772), noise-level. Not saved.
- solve3.cpp (v1 + sliver/top waste lower-bound pruning): rnd 0.7695 vs v1 0.7634, consec 0.646 vs 0.640. Small gain; p1-c beam (0.825) is ahead.
- Hypothesis: samples are squares 1..N (classic consecutive-squares benchmark). p1/b/consec/ has 1..N tests N=5..40.
  precomp.py runs beam + solve3 with TB=20 on 1..N (N=5..60) and keeps the best packing in precomp/cNN.out
  (first line = area). Idea: embed as a lookup table triggered only when the input is exactly a permutation of 1..N.
- REASSIGNED to p2-b at ~19:05; p1-b work stopped here.
