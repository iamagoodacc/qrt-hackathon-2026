# p3 ideas from p3-c (for p3-b, who owns Run Code / the experiment order)

## Insurance (orchestrator can SUBMIT now): p3/c/safe.cpp
x = t for t <= 0.08 (echo measured +0.002..+0.0035), 1000 x 0.05 for t >= 0.82 (measured +0.0079 at
t=1.00), exactly 0 elsewhere. Small but non-zero official score. Replace once a better candidate is confirmed.

## Independent fit (p3/c/fit.py, data = round 0 echo)
Model per t: s(x) = a x - c x^2. With one point per t the optimum u(t) needs f = share of the target a
constant can earn (unknown: notes say ~1/8 at small t, ~1/4 at t=1). Echo x=t OVERSHOOTS for t>=0.10, so:
  u(t) for f = 1/8 (conservative) ~ 0.045 (t 0.10-0.18), 0.05->0.09 (0.20-0.48), 0.13->0.23 (0.50-0.78),
  ~0.07-0.10 (0.80-1.00).  f = 1 gives ~1.4-2x larger values.
Candidate: p3/c/xstar.cpp = table x*(t) at f=1/8, rolling-median smoothed (N=1). If the true f is larger,
this sits at z = x/u ~ 0.5-0.7 of the optimum -> still ~75-90% of the best constant payoff, never negative.

## Suggested probes (each RUN CODE = 51 readings, one per t)
1. RUN p3/c/xstar.cpp. Expect positive in every regime. A region that is still negative => u is smaller there.
   Log: `python3 p3/parse.py --probe file:p3/c/xstar.cpp < paste.txt`
2. RUN p3/c/xstar_half.cpp (0.5 x*). Gives a 2nd/3rd x per t; then `python3 p3/c/fit.py 0` fits a, c per t
   exactly (no f assumption). Log: `python3 p3/parse.py --probe "file:p3/c/xstar.cpp*0.5" < paste.txt`
3. N test (why constants earn only ~1/8 of target): same x*(t) but N=2 and N=1000 copies. If the payoff
   scales with N the box sums per element; if not, it averages. Then test SHAPE, e.g. [x, 0, ..., 0] vs
   [0, ..., 0, x] vs [x, -x, x, -x]: the target likely needs a non-constant vector (time series / per-asset).
4. Noise: heavy tails (single +0.12..+0.17 readings at t=0.56/0.68/0.70 among strongly negative neighbours).
   Never pick the final candidate from one run; compare medians over neighbouring t and repeats.
5. FINAL submission must be the best candidate by NOISELESS expectation (official score = last submission).

## After run 2 (p3-c joint fit of runs 1+2, quadratic per regime, 20:30)
A x*=+0.04 | B slope<0, x*~-0.13 (extrapolated) | 0.50-0.52 like B | C x*=+0.31 (s*~50% target) | D slope<0, x*~-0.4.
Candidate: p3/c/final2.cpp (A 0.04, B -0.06, 0.50-0.52 -0.06, C 0.31, D -0.15).
If ONE more Run Code fits: multiplex negatives by case index k%3 in B and D:
  B: x in {-0.03, -0.08, -0.15};  D: x in {-0.05, -0.15, -0.30};  C: x in {0.25, 0.31, 0.40}.
