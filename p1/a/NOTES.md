# P1 builder

- Place each input square once, in input order in the returned coordinate list.
- Integer top-left coordinates, axis-aligned sides; touching edges is allowed.
- Square interiors must not overlap. Negative coordinates are allowed by the statement.
- Minimize the bounding rectangle area, using coordinate extrema (translation invariant).
- Function: `vector<vector<int>> pack_squares(vector<int> squares)`; local main only under LOCAL.

Exact score: `(L/A)^20`, with `S=sum(s*s)`, `m=max(s)`,
`L=min_{integer W>=m} W*max(m,ceil(S/W))`. Invalid outputs score zero.

Limits: number of squares and maximum side are NOT PROVIDED. P0 used n<=100, side<=30;
these are provisional test scales, not asserted P1 limits. Include larger robustness cases.
Time limit: NOT PROVIDED. Use provisional 2 seconds; any search budget <=1.5 seconds.

Plan: independently audit reusable P0 checker, implement exact bound and strict output parsing,
cover all four diagram examples and invalid cases, generate reproducible mixed tests, then
save a simple deterministic descending shelf baseline and stop (builder role).

Checker/tests ready: 35 deterministic cases, including four diagram examples, 24 uniform
random cases over the full provisional ranges, and explicit robustness/structured cases.
Actual maximum constraints remain unknown. Samples are included in the local average;
HackerRank excludes samples, so the local mean is not a calibrated hidden-score estimate.
Checker regression script passed four samples, translation, edge touching, malformed and
overlapping output, large coordinates, and 2000 brute-force lower-bound comparisons.

NEW BEST score=0.3191 (best 0.0000) max_time<0.01s idea=baseline descending next-fit shelves with sampled widths
All 35 tests positive. Compiled both LOCAL executable and no-main HackerRank object.
Saved via harness to best/p1/BEST.cpp. Builder work complete; waiting for further assignment.
