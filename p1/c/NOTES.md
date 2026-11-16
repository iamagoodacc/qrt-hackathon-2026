# p1-c notes (second optimiser)

## Rules
- Squares with integer sides a_i; place each axis-aligned at integer top-left (X, Y), X down, Y right.
- Square i occupies [X, X+s) x [Y, Y+s); interiors must not overlap (touching OK).
- Return one {X, Y} per square in INPUT order via `vector<vector<int>> pack_squares(vector<int>)`.
- Minimise the area A of the bounding rectangle.

## Scoring
m = max side, S = sum s^2, L = min_{W>=m} W * max(m, ceil(S/W)).  Score = (L/A)^20. Invalid = 0.
1% extra area -> 0.82, 0.5% -> 0.905, 0.1% -> 0.98.

## Limits
Constraints and time limit NOT given yet. Handle any n / side (segment skyline, no fixed arrays).
Assume 2 s, budget ~1.5 s.

## Checker audit (p1/checker.py vs statement) -- no mismatch found
- Overlap: strict inequalities on half-open rects -> touching allowed. OK.
- Bounding box from coordinate extrema (translation invariant). OK.
- lower_bound (optimised loop) == brute-force formula on 20,000 random inputs. OK.
- Samples: [1],[1,2],[1,2,3] score 1.0; [1,2,3,4] diagram answer 5x7=35 vs L=30 -> 0.046 (correct by formula).
- Only caveat: checker allows negative coords; we always output min coord 0 anyway.

## Approach (different from b's DFS restarts)
Beam search over skyline states inside a fixed W x H container: layer = one move (place a square
at the lowest gap against the taller neighbour, or waste the gap). Key = waste (+ forced waste of
dead slivers) - progress, dedupe identical states. Container list shrinks like p0-c.

## Log
- NEW BEST score=0.8135 (best 0.7761) max_time=1.50s idea=beam.cpp: beam search over skyline in fixed WxH container, key=waste+forced sliver waste-0.02*placed, B 8->512, most-slack containers first
- tuning (p1/c/tune, 4 jobs, not saved): W_AREA 0.15 best (0.8216 vs 0.7988 at 0.02). Endgame DFS (ER=8/12) WORSE.
- UNSAVED FINDING: cap container width W <= 3*m (env WF=3 / CFG::W_MAX_FACTOR) -> 0.8539 on tune set (WF=4 0.8445, WF=6 0.8180, no cap 0.8216).
  Next step for whoever continues: set W_MAX_FACTOR=3 default, try 2/2.5, run harness --save. (p1-c reassigned to p2 at this point.)
