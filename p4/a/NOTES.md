# P4 builder
- Return exactly R rows of C characters from .-|LJF7.
- Used cells form exactly one closed non-self-crossing orthogonal loop; each has degree 2.
- Visit every pearl. White: straight, with a turn in at least one neighboring loop cell.
- Black: turn, with both neighboring loop cells straight.
- Empty cells may be used or skipped; adjacent used cells need not connect.

Exact score fraction: min(1, s*L_ref/L)^2 * (0.5+0.5*exp(-max(0,T-1)/5)).
Actual organiser references unavailable: fixed local witness references use s=1, stored alongside
tests. They are feasible upper bounds, not necessarily optimal; never change during comparisons.
Constraints: 6<=R,C<=50, at least 2 pearls, solvable. 100 hidden cases; time limit 10s.
Baseline budget <=8s inside function. Prioritise feasibility. Function vector<string> solve(vector<string>).
Plan: independent loop validator, reference-aware scoring, generated witness loops, simple rectangle
construction plus mask-CSP search baseline, then stop.

Checker/tests ready: 27 solvable cases, uniform random R,C across 6..50, explicit 50x50 and
6x50, rectangular and top-indented witnesses, plus statement sample. Fixed .ref files and
references.json index contain witness lengths (sample optimum18). All witnesses checker-verified.
Test checker covers output dimensions/alphabet, broken links, missed pearls, corner/straight
pearl violations, multiple loops, speed factor, and all generated witnesses.

First unsaved mask-CSP baseline: score=0.0643, zero=4, max_time=8.01s. Validity failures were
search exhaustion; nothing saved. Added direct rectangle/top-notch geometric construction
before generic mask-domain propagation and DFS. Time starts inside solve's per-call Solver.

NEW BEST score=0.9878 (best 0.0000) max_time<0.01s idea=baseline geometric loops with mask-CSP fallback
All 27 tests valid. Built LOCAL C++17, no-main C++17, and actual HackerRank C++23 hidden main;
scored hidden-main binary before save. best/p4/BEST.cpp saved by harness.

IMPORTANT: local 0.9878 is NOT an expected HackerRank score. Generated puzzles are deliberately
simple known-feasible rectangle/notch families and references are feasible witness upper bounds;
26/27 hit capped quality, while the sample gets ~0.669 (22-cell loop versus optimal18). Real dense
and irregular puzzles require substantially better search and broader tests. Fallback is incomplete
within the time budget and returns an empty loop on failure; no general validity guarantee.
Builder work complete; stopping and waiting, no optimisation performed after valid baseline.

Expanded shared tests at user request: imported all 10 p4/hard/*.in unchanged and generated
10 extra spanning sizes 10..50, pearl_prob 0.15..1.0. Total now 47. Every added .sol is a
validated Hamiltonian/tree-outline witness, and .ref length equals its exact cell count with s=1.
Original hard cases lacked witnesses; deterministic seed recovery found compatible witnesses
(seeds 11,12,1,2,3,4,5,6,7,13 in filename order), independently validated against each puzzle.
Reproduce imports/extras: python3 p4/a/add_hard_tests.py. gen_hard.puzzle now optionally
returns witness, and has import-safe main guard; its original CLI output is preserved.
p4/gen.py now preserves existing reference entries, so regenerating easy tests retains hard refs.
Validation: regenerated easy suite, all checker regressions passed, all 47 .sol files score1,
all .ref files equal reference index metadata, imports byte-identical. No solver/best changed.
IMPORTANT: pre-expansion aggregate SCORE is stale and incomparable; rerun candidates on all47.
