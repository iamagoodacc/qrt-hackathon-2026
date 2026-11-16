# p0-c notes (second optimiser)

## Rules
- n squares, integer sides; place each axis-aligned at integer (X, Y) = top-left; X down, Y right.
- Square i occupies [X, X+s) x [Y, Y+s). Interiors must not overlap (touching OK).
- Output exactly n lines "X Y" in input order. Any integers (negatives OK).
- Minimise area A of the bounding rectangle of all squares.

## Scoring
m = max side, S = sum s^2, L = min_{W>=m} W * max(m, ceil(S/W)).  Points = (L/A)^20. Invalid = 0.

## Limits
1 <= n <= 100, 1 <= side <= 30. Time limit 2 s per test (budget ~1.6 s).

## Checker audit (p0/checker.py vs statement)
- Overlap test uses strict inequalities on half-open rects: matches "touching is fine".
- Bounding box = max(x+s)-min(x) by max(y+s)-min(y): matches.
- L: loops W in [m, ceil(S/m)]; larger W gives height m and larger area, so the range is enough. OK.
- Line count via splitlines(): a trailing BLANK line would be rejected (HackerRank probably
  tolerates it). Conservative, harmless as long as we print exactly n lines.
- Integers: regex allows leading +/-. OK. elapsed_s unused (statement has no time penalty). OK.
- Verdict: no mismatch found.

## My approach (not SA)
Fixed-container decision search: for a container W x H (area below current best), fill with a
skyline "lowest-leftmost gap" DFS: at the gap, try each distinct remaining size that fits
(big first), or waste the gap up to its lower neighbour. Prune when wasted area > W*H - S.
Iterate W over a window, H = floor((best-1)/W), with node limits + randomised restarts.

## Log
- NEW BEST score=0.7550 (best 0.1597) max_time=1.67s idea=skyline DFS in fixed container W x H, shrink target area (most-slack container first, doubling node limits)
- experiment: 10 s instead of 1.6 s only gains ~0.3% area on a few tests -> chronological DFS gets stuck; need better search (restarts/LDS/ordering), not more time.
- v2 (solve2.cpp): candidate order = size + 100 if exact gap fit + 50 if top level with neighbour, noisy restarts (noise 3) -> ~0.79 (parallel eval). Bonuses matter (+0.045).
- FAILED: subset-sum lookahead bonus (leftover gap width reachable) halves node rate, 0.73-0.78.
- NEW BEST score=0.8188 (best 0.8065) max_time=1.61s idea=solve4.cpp: place square against the TALLER neighbour (Burke best-fit), noise 4, node cap 50k
- Observation: winning containers are narrow strips (width ~1.5-2.5 x max side). Waste = 1-wide slivers + ragged top rows.
