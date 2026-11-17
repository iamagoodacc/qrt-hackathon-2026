"""Append HackerRank black-box feedback to p3/data.csv.

Usage:  python3 p3/parse.py --probe "echo" < pasted.txt
        pbpaste | python3 p3/parse.py --probe "const:0.05" --n 1000
Finds every line containing key=value pairs like
  t=0.56, N=1, raw_noisy_score=0.1180, raw_noiseless_target=0.1352, normalized_noisy_score=...
and writes one CSV row per line. --probe says what the submission returned so x can be recorded:
  echo          -> x = t
  scale:k       -> x = k*t
  const:v       -> x = v
  file:p3/c/xstar.cpp*0.5 -> x = 0.5 * (table X in that file at index round(t/0.02))
  anything else -> x left blank (the label is still stored)
Writes the existing p3/data.csv format: round,case,t,N,x,raw_noisy,target
(round = next integer unless --round given; case = 1-based order of the line in the paste).
"""
import argparse, csv, re, sys
from pathlib import Path

CSV = Path(__file__).resolve().parent / "data.csv"
COLS = ["round", "case", "t", "N", "x", "raw_noisy", "target"]
NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"

def x_for(probe, t):
    if probe.startswith("file:"):      # file:path.cpp[*k] -> table X = {...} indexed by round(t/0.02)
        spec = probe[5:]; k = 1.0
        if "*" in spec: spec, k = spec.split("*"); k = float(k)
        body = Path(spec).read_text()
        X = [float(v.rstrip("f")) for v in re.search(r"X\s*=\s*\{([^}]*)\}", body).group(1).replace(" ", "").split(",") if v]
        return X[max(0, min(len(X) - 1, round(t / 0.02)))] * k
    if probe == "echo": return t
    if probe.startswith("scale:"): return float(probe[6:]) * t
    if probe.startswith("const:"): return float(probe[6:])
    return ""

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--probe", required=True, help="what the submission returned (see doc)")
    ap.add_argument("--round", default="", help="round number (default: max existing + 1)")
    ap.add_argument("--n", default="", help="array length if the message lacks N")
    ap.add_argument("--dry", action="store_true", help="print rows, do not write")
    a = ap.parse_args()
    existing = list(csv.DictReader(CSV.open())) if CSV.exists() else []
    rnd = a.round or str(max([int(r["round"]) for r in existing if r.get("round", "").lstrip("-").isdigit()] or [-1]) + 1)
    rows = []
    for line in sys.stdin:
        kv = dict(re.findall(r"([A-Za-z_]+)\s*=\s*(" + NUM + ")", line))
        if "t" not in kv or "raw_noisy_score" not in kv: continue
        t = float(kv["t"])
        x = x_for(a.probe, t)
        rows.append({"round": rnd, "case": len(rows) + 1, "t": f"{t:.2f}", "N": kv.get("N", a.n),
                     "x": f"{x:.4f}" if x != "" else a.probe, "raw_noisy": kv["raw_noisy_score"],
                     "target": kv.get("raw_noiseless_target", "")})
    if a.dry:
        for r in rows: print(r)
        return
    new = not CSV.exists()
    with CSV.open("a", newline="") as f:
        w = csv.DictWriter(f, fieldnames=COLS)
        if new: w.writeheader()
        w.writerows(rows)
    print(f"appended {len(rows)} rows (round {rnd}, probe={a.probe}) to {CSV}")

if __name__ == "__main__":
    main()
