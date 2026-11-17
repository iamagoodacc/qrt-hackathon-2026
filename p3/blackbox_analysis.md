# Black box (p3) - findings so far (from teammate's runs, pasted by the orchestrator)

Run code with `vector<float> probe(float t) { return {t}; }` (x = t, N = 1).
Run Code shows 51 cases: t = 0.00, 0.02, ..., 1.00 (case k has t = 0.02*(k-1)).
Checker message per case (shown as "Compiler Message"):
`t=<t>, N=<array length>, raw_noisy_score=<s>, raw_noiseless_target=<T>, normalized_noisy_score=clip(s/T, 0, 1)`
Official score = NOISELESS score of the FINAL submission. Noisy feedback is only a guide.

| t | raw_noisy_score | target |
|---|---|---|
| 0.00 | 0.0000 | 0.0148 |
| 0.02 | 0.0035 | 0.0131 |
| 0.04 | 0.0022 | 0.0140 |
| 0.06 | 0.0035 | 0.0162 |
| 0.08 | 0.0034 | 0.0204 |
| 0.10 | -0.0006 | 0.0244 |
| 0.12 | -0.0055 | 0.0254 |
| 0.14 | -0.0097 | 0.0248 |
| 0.16 | -0.0281 | 0.0238 |
| 0.18 | -0.0316 | 0.0253 |
| 0.20 | -0.0412 | 0.0393 |
| 0.22 | -0.0713 | 0.0587 |
| 0.24 | -0.0915 | 0.0754 |
| 0.26 | -0.1343 | 0.0886 |
| 0.28 | -0.1144 | 0.0959 |
| 0.30 | -0.1592 | 0.0969 |
| 0.32 | -0.0834 | 0.0915 |
| 0.34 | -0.1507 | 0.0867 |
| 0.36 | -0.1310 | 0.0892 |
| 0.38 | -0.1507 | 0.0986 |
| 0.40 | -0.2021 | 0.1104 |
| 0.42 | -0.1540 | 0.1202 |
| 0.44 | -0.2643 | 0.1318 |
| 0.46 | -0.2466 | 0.1274 |
| 0.48 | -0.2696 | 0.1059 |
| 0.50 | -0.0762 | 0.0925 |
| 0.52 | -0.0626 | 0.1109 |
| 0.54 | -0.0816 | 0.1334 |
| 0.56 | +0.1180 | 0.1352 |
| 0.58 | -0.0643 | 0.1250 |
| 0.60 | -0.0638 | 0.1229 |
| 0.62 | -0.0925 | 0.1240 |
| 0.64 | -0.0655 | 0.1278 |
| 0.66 | -0.0712 | 0.1308 |
| 0.68 | +0.1750 | 0.1274 |
| 0.70 | +0.0977 | 0.1291 |
| 0.72 | -0.0627 | 0.1397 |
| 0.74 | -0.0758 | 0.1387 |
| 0.76 | -0.0581 | 0.1174 |
| 0.78 | -0.2568 | 0.0853 |
| 0.80 | -0.0162 | 0.0524 |
| 0.82 | -0.3477 | 0.0284 |
| 0.84 | -0.1854 | 0.0203 |
| 0.86 | -0.2672 | 0.0220 |
| 0.88 | -0.6151 | 0.0247 |
| 0.90 | -0.3129 | 0.0249 |
| 0.92 | -0.5308 | 0.0253 |
| 0.94 | -0.7101 | 0.0269 |
| 0.96 | -0.2743 | 0.0290 |
| 0.98 | -0.2841 | 0.0308 |
| 1.00 | -0.3025 | 0.0324 |

## Earlier findings (teammate, mostly small t)
- N = length of returned array. 1000 values work fine.
- Constant 0 -> exactly 0.0000 (any N). Positive constants lose ~ value^2 (1.0 -> about -0.46 at t~0.05).
  Negative constants lose ~ |value| (-0.3 -> -0.043, -0.1 -> -0.015, -0.03 -> -0.003).
- Best constants at small t earn only ~1/8 of target. Larger N did NOT reduce noise (+-0.004 at small t).
- Ramp up / ramp down / alternating 0,0.1: no clear effect of order.
- t=1.00: constant 0.05 x1000 -> 0.0079 (24% of target); x200 -> 0.0031 (so N may matter there).
- Untested: does position in the array matter ([0.1]*100+[-0.1]*100 vs flipped)?

## Teammate's interpretation
- Target depends strongly on t, in regimes: small (~0.015-0.025) for t<=0.18, ~0.09-0.14 for t in
  [0.24, 0.76], collapses to ~0.02-0.03 for t>=0.82. Boundaries near t=0.48/0.50 and 0.78/0.80.
- Small x is safe, large x loses roughly quadratically (curvature varies by regime).
- Noise is heavy-tailed: occasional big positive jumps (cases 29, 35, 36). Single readings mislead.
