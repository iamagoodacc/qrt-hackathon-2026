# p0-b notes (main optimiser)

Rules
- n squares (1<=n<=100, sides 1..30), place on integer grid, axis-aligned, no overlap (touching ok).
- Output n lines "X Y" (top-left, X down, Y right), input order.
- Minimise bounding-box area A.

Scoring: m=max side, S=sum s^2, L=min_{W>=m} W*max(m,ceil(S/W)); points=(L/A)^20. Invalid=0.
L is basically S (tests: L/S <= 1.006), so we need near-perfect tilings: 1% waste ~ 0.82.

Time limit: 2 s per test -> budget ~1.6 s.

Plan
- Equal sizes are interchangeable: state = count per size.
- Decoder: best-fit skyline (lowest gap, largest square that fits) inside a W x H box,
  abort when waste > W*H - S. Initial: uncapped strip packing over all widths.
- Search: try targets (W,H) with area < best, randomised choices / SA on choice vector.

## Log
- NEW BEST score=0.8065 (best 0.7550) max_time=1.60s idea=best-fit skyline in WxH box, random decodes, targets by area asc
- score=0.7973 (best 0.8065) max_time=1.60s idea=+SA over decision genome (rank/side/raise), 25% phase1 -> worse
