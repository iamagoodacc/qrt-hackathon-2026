#!/usr/bin/env python3
"""python3 p4/b/peval4.py <binary> <testdir> [jobs] [ENV=VAL ...]; ref from NN.ref (witness length)"""
import sys, os, subprocess, time, math, importlib.util
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p4/checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
binary, tdir = sys.argv[1], Path(sys.argv[2]); jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 8
env = dict(os.environ)
for kv in sys.argv[4:]: k, v = kv.split("="); env[k] = v
def run(t):
    inp = t.read_text(); ref = int(t.with_suffix('.ref').read_text().split()[0]); t0 = time.time()
    try: p = subprocess.run([binary], input=inp, capture_output=True, text=True, env=env, timeout=10)
    except subprocess.TimeoutExpired: return t.name, 0.0, 10.0, "TIMEOUT"
    el = time.time() - t0
    try:
        L = chk.validate(chk.parse_input(inp), p.stdout)
        sc = min(1.0, ref / L) ** 2 * (0.5 + 0.5 * math.exp(-max(0.0, el - 1.0) / 5.0)); info = f"L={L} ref={ref}"
    except Exception as e: sc = 0.0; info = f"INVALID {e}"
    return t.name, sc, el, info
with ThreadPoolExecutor(jobs) as ex: res = list(ex.map(run, sorted(tdir.glob("*.in"))))
for name, sc, el, info in res: print(f"{name} {sc:.3f} {el:.2f}s {info}")
print(f"MEAN {sum(r[1] for r in res)/len(res):.4f} n={len(res)} max_t={max(r[2] for r in res):.2f}")
