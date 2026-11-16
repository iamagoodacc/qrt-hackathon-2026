#!/usr/bin/env python3
"""Offline: long runs on squares 1..N (ascending input), keep best output per N in precomp/.
python3 p1/b/precomp.py NMIN NMAX TB JOBS"""
import sys, os, subprocess, importlib.util
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
HERE = Path(__file__).resolve().parent; ROOT = HERE.parents[1]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p1/checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
nmin, nmax, tb, jobs = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3], int(sys.argv[4])
bins = [str(ROOT / "p1/c/beam"), str(HERE / "solve3")]
def area(inp, out):
    s = list(range(1, len(out.splitlines()) + 1))
    c = [tuple(map(int, l.split())) for l in out.splitlines()]
    return (max(x + a for (x, y), a in zip(c, s)) - min(x for x, y in c)) * (max(y + a for (x, y), a in zip(c, s)) - min(y for x, y in c))
def run(job):
    N, b = job
    inp = f"{N}\n" + "\n".join(map(str, range(1, N + 1))) + "\n"
    env = dict(os.environ, TB=tb)
    p = subprocess.run([b], input=inp, capture_output=True, text=True, env=env)
    try:
        chk.score(inp, p.stdout)
    except Exception as e:
        return N, None
    a = area(inp, p.stdout)
    f = HERE / "precomp" / f"c{N:02d}.out"
    old = int(f.read_text().split("\n", 1)[0]) if f.exists() else 10**18
    if a < old:
        f.write_text(f"{a}\n{p.stdout}")
    return N, a
jobs_l = [(N, b) for N in range(nmin, nmax + 1) for b in bins]
with ThreadPoolExecutor(jobs) as ex:
    for N, a in ex.map(run, jobs_l): print(N, a, flush=True)
