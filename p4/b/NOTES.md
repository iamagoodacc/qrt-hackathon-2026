# p4-b notes (main optimiser)

## Rules
- R x C grid (6..50), '.', 'W', 'B'. Draw one closed loop through cell centres (orthogonal moves),
  no cell twice, must visit every pearl. Output chars . - | L J F 7 per cell.
- White: loop goes straight through it AND turns in at least one of the two loop-neighbours.
- Black: loop turns in it AND goes straight in both loop-neighbours.

## Scoring
score = 100 * quality * speed; quality = min(1, s*Lref/L)^2 (s=1 mostly, 0.9 on a few large);
speed = 0.5 + 0.5*exp(-max(0, T-1)/5). Invalid = 0. Time limit 10 s (timeout = 0).

## Plan
Edge CSP: edge state unknown/on/off, propagate degree (0 or 2; pearls 2), black/white rules,
no premature sub-loop (chain partner endpoints), DFS branching at the chain end with fewest options.
First valid loop fast; then randomised restarts / B&B on loop length within ~1 s.

## Log
- NEW BEST score=1.0000 (best 0.9878) max_time=0.94s idea=solve2.cpp: edge CSP + longest-chain-focus DFS + distance ordering + B&B restarts, p4-a geometric loops as start
- Hard private set p4/b/tests_hard (hardgen.py: random winding loop via bump moves, pearls on qualifying
  cells, .ref = witness length, .sol = witness; peval4.py scores them). Saved BEST (FOCUS=1, no CONN): 0.19.
- solve2.cpp now defaults FOCUS=0 (most-constrained chain end) + CONN=1 (connectivity prune): hard 0.454,
  shared tests still 1.0 (geometric covers them). NOT saved by harness (shared tests already 1.0) ->
  ORCHESTRATOR: submit p4/b/solve2.cpp, it is strictly better on realistic puzzles.
- FAILED: failed-literal probing (root + branch cell): hard 0.26-0.29, worse (too slow). Disabled (PROBE_*=0).
- Still failing: large dense puzzles (loops 600-1000 cells). Next ideas: restart diversity (alternate FOCUS
  per restart with CONN), cheaper probing only at root with a time cap, or path-based DFS for dense grids.
