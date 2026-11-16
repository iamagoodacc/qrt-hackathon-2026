# p4-c notes (second optimiser)

## Rules
- R x C grid (6..50 each) with '.', 'W', 'B'. Draw ONE closed loop through cell centres (orthogonal
  moves, no crossing, each cell at most once) visiting EVERY pearl.
- White: straight through, and turns in at least one of the two neighbouring loop cells.
- Black: turns in the cell, and goes straight in both neighbouring loop cells.
- Output R rows: '.', '-', '|', 'L'(N+E), 'J'(N+W), 'F'(S+E), '7'(S+W). Function `vector<string> solve(vector<string>)`.

## Scoring
score = 100 * quality * speed; quality = min(1, s*Lref/L)^2 (s=1 mostly, 0.9 on a few large);
speed = 0.5 + 0.5*exp(-max(0,T-1)/5). Invalid = 0. Locally Lref = fixed per-test ref file.
T=1s free; 2s -> 0.91; 3s -> 0.84.

## Limits
6 <= R, C <= 50, >= 2 pearls, always solvable. 100 hidden tests increasing size/difficulty.
Time limit 10 s (timeout = 0). Aim to finish in ~1 s; use more only when it helps.

## Plan
Wait for checker/tests, audit checker. Then an approach different from b's.
- Checker audit (p4/checker.py vs statement): no mismatch. Masks N1 E2 S4 W8 (L=N+E, J=N+W, F=S+E, 7=S+W)
  correct; mutual connections, pearls visited, white = straight + >=1 turning neighbour, black = turn +
  both neighbours straight, single loop via connectivity of used cells; score = quality*speed, fixed refs.
## Log
- solve.cpp: region SA (flip dual squares, Yokoi simple-point test, no diagonal pinch, cost len+4*violations,
  40 restarts, 0.85 s). Shared tests 0.9863 (best 1.0 = b; shared tests are easy rectangles, refs = witnesses).
- p4/c/tests_hard (mygen.py: random simply-connected regions, pearl density 0.05-0.8, ref = witness length):
  best/p4/BEST.cpp (b) 0.5226 with 11 INVALID; my SA 0.6795 with 7 INVALID (b fails on all of those too).
- combo.cpp = b's solve2 (frozen copy b_solve2_frozen.cpp) + my SA: SA 0.4 s -> CSP (short deadline if SA
  found a loop, else up to 7.5 s) -> SA again seeded from CSP loop -> shortest valid.
  Hard set 0.6967 (6 INVALID: dense 00,03,07,18,20,23), shared tests 0.9911 (max 1.13 s). NOT saved (harness
  best 1.0 on easy shared tests). RECOMMEND orchestrator add tests_hard to p4/tests and consider combo.cpp.
- Left to do: dense puzzles (density 0.4-0.8, big loops) still unsolved by both CSP and SA -> CSP needs
  stronger propagation; test 17 spends 8 s (SA phase A missed, CSP slow) -> run SA longer before CSP fallback.
