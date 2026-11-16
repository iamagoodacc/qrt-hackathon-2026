# P2 builder
- Return square N by N grid, filled only with uppercase A-Z.
- Every required word occurs along at least one of 8 directions, wrapping modulo N.
- No forbidden word occurs at any start or direction, also wrapping modulo N.
- N must be at least the longest required word, even if periodic wrapping could fit it.
- Output uses N rows with single-space-separated letters, matching HackerRank hidden main.

Exact harness score: min(1, sum(required word lengths)/(1.7*N*N)). Invalid scores zero.
Duplicate required words contribute to S as listed; a single occurrence satisfies duplicates.
Constraints on word counts and lengths are missing. No finite maximum can be claimed.
Time limit missing: provisional 2s, solution budget 1.5s. Forbidden words longer than N
are checked with repeated wraps, per the statement's explicit provisional instruction.
Input: R, R words, F, F words (whitespace separated is equivalent).
Plan: exact checker, known-feasible reproducible tests, randomized valid greedy baseline; stop.

Checker and 31 reproducible tests ready. Four regression groups cover supplied example,
diagram, all 8 wrapping directions, multiwrap forbidden words, minimum N, malformed output,
and duplicate-required score cap. Every generated test has an independently checker-verified
witness. Random words use provisional counts 1..40, lengths 3..12; forbidden counts 0..20.
Random forbidden words contain Y while required words omit Y, guaranteeing feasibility;
optimisers should add broader forbidden distributions once actual limits are available.
Sample and diagram are in local average, while HackerRank excludes samples.

NEW BEST score=0.0820 (best 0.0000) max_time<0.01s idea=baseline separated required rows with verified filler
All 31 tests valid and positive. Compiled C++17 LOCAL and no-main object, then compiled against
actual hr_main.cpp via hrbuild.sh and scored that binary before saving. Baseline is deliberately
sparse. General hard instances may require intersecting words to avoid forbidden matches;
if all construction attempts fail it has no guaranteed valid fallback. Optimisers should replace it.
Saved via harness to best/p2/BEST.cpp. Builder complete; stopped and waiting.
