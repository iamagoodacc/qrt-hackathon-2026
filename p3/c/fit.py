"""p3-c independent fit of p3/data.csv. Model per t: payoff s(x) = a*x - c*x^2 (s(0) = 0 exactly).
 - >= 2 distinct numeric x near t (window W): least squares for a, c -> x* = a/(2c), s* = a^2/(4c).
 - only 1 point near t (echo data so far): s/Teff = 2z - z^2 with z = x/u, Teff = f*target
   (f = share of target the best CONSTANT can earn; notes say ~1/8 at small t, ~1/4 at t=1).
   s < 0 => x overshoots: z = 1 + sqrt(1 - s/Teff), u = x/z.
Usage: python3 p3/c/fit.py [W=0.03] [f list, default 1,0.25,0.125]"""
import csv, math, sys
from pathlib import Path
W = float(sys.argv[1]) if len(sys.argv) > 1 else 0.03
FS = [float(v) for v in sys.argv[2].split(",")] if len(sys.argv) > 2 else [1.0, 0.25, 0.125]
rows = []
for r in csv.DictReader((Path(__file__).resolve().parents[1] / "data.csv").open()):
    try: rows.append((float(r["t"]), float(r["x"]), float(r["raw_noisy"]), float(r["target"]), r["N"]))
    except ValueError: pass
ts = sorted({round(t, 2) for t, *_ in rows})
print(f"{'t':>5} {'target':>7} {'n':>3} " + " ".join(f"u(f={f:g})".rjust(10) for f in FS) + "   LS x*  (if >=2 x)")
out = []
for t0 in ts:
    near = [r for r in rows if abs(r[0] - t0) <= W + 1e-9]
    T = sum(r[3] for r in near if abs(r[0] - t0) < 1e-9) / max(1, sum(1 for r in near if abs(r[0] - t0) < 1e-9))
    xs = {round(r[1], 4) for r in near}
    ls = ""
    if len(xs) >= 2:   # least squares on s = a x - c x^2
        Sxx = sum(r[1] ** 2 for r in near); Sx3 = sum(r[1] ** 3 for r in near); Sx4 = sum(r[1] ** 4 for r in near)
        Sxs = sum(r[1] * r[2] for r in near); Sx2s = sum(r[1] ** 2 * r[2] for r in near)
        det = Sxx * Sx4 - Sx3 * Sx3
        if abs(det) > 1e-15:
            a = (Sxs * Sx4 - Sx2s * Sx3) / det; c = -(Sxx * Sx2s - Sx3 * Sxs) / det
            ls = f"x*={a / (2 * c):.4f} s*={a * a / (4 * c):.4f}" if c > 0 else f"not concave a={a:.3f} c={c:.3f}"
    us = []
    for f in FS:
        vals = []
        for t, x, s, Tt, N in near:
            Te = f * Tt
            if x <= 0 or Te <= 0: continue
            q = 1 - s / Te
            if q < 0: q = 0.0           # noisy reading above Teff: treat as at the optimum
            z = 1 + math.sqrt(q) if s < 0 else 1.0   # s >= 0 and single point: ambiguous, assume near optimum
            vals.append(x / z)
        vals.sort()
        us.append(vals[len(vals) // 2] if vals else float("nan"))   # median over the window
    out.append((t0, T, us))
    print(f"{t0:5.2f} {T:7.4f} {len(near):3d} " + " ".join(f"{u:10.4f}" for u in us) + "   " + ls)
