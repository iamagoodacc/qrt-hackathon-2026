#!/usr/bin/env python3
"""Parallel local eval: python3 p1/b/peval.py <binary> <testdir> [jobs] [ENV=VAL ...]"""
import sys, os, subprocess, time, importlib.util
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p4/checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
binary, tdir = sys.argv[1], Path(sys.argv[2])
jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 8
env = dict(os.environ)
for kv in sys.argv[4:]:
    k, v = kv.split("="); env[k] = v
tests = sorted(tdir.glob("*.in"))
def run(t):
    inp = t.read_text(); t0 = time.time()
    p = subprocess.run([binary], input=inp, capture_output=True, text=True, env=env, timeout=12)
    el = time.time() - t0
    try: sc = chk.score(inp, p.stdout, el)
    except Exception as e: sc = 0.0; print(t.name, "INVALID", e)
    return t.name, sc, el, p.stderr.strip().splitlines()[-1] if p.stderr.strip() else ""
with ThreadPoolExecutor(jobs) as ex: res = list(ex.map(run, tests))
for name, sc, el, err in res:
    if "-v" in os.environ.get("PV", ""): print(f"{name:>16} {sc:.3f} {el:.2f}s {err}")
print(f"MEAN {sum(r[1] for r in res)/len(res):.4f} n={len(res)} max_t={max(r[2] for r in res):.2f}")
