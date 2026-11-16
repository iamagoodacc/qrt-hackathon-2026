"""Parallel evaluator (p1-c helper). Usage: python3 p1/c/peval.py ./p1/c/beam [testdir] [jobs]
Scores with p1/checker.py; prints per-test score, area/L and the mean. Does NOT save anything."""
import sys, subprocess, time, importlib.util
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p2" / "checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
cmd = sys.argv[1]
tdir = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "p2" / "tests"
jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 8
tests = sorted(tdir.glob("*.in"))
def run(t):
    inp = t.read_text(); st = time.perf_counter()
    p = subprocess.run(cmd, shell=True, input=inp, capture_output=True, text=True, cwd=ROOT)
    el = time.perf_counter() - st
    try:
        sc = chk.score(inp, p.stdout)
        req, fb = chk.parse_input(inp); g = chk.parse_output(p.stdout)
        S = sum(map(len, req)); import math
        sc_info = f"N={len(g)} S={S} Nlo={max(max(map(len,req)), math.ceil(math.sqrt(S/1.7)))} dens={S/len(g)**2:.3f}"
    except Exception as e:
        sc, sc_info = 0.0, f"INVALID {e}"
    return t.name, sc, el, sc_info, p.stderr.strip()[-120:]
with ThreadPoolExecutor(jobs) as ex:
    res = list(ex.map(run, tests))
for name, sc, el, info, err in res:
    print(f"{name:>18} {sc:6.3f} {el:5.2f}s {info} | {err}")
print(f"MEAN {sum(r[1] for r in res)/len(res):.4f} n={len(res)} max_time={max(r[2] for r in res):.2f}")
