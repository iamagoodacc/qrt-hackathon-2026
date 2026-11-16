"""Parallel p4 evaluator (p4-c). python3 p4/c/peval.py BIN [testdir] [jobs]. Uses p4/checker.py validate;
score vs <test>.ref next to the test (fixed). Prints per-test L/ref and mean."""
import sys, subprocess, time, json, math, importlib.util
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p4" / "checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
cmd = sys.argv[1]; tdir = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "p4" / "tests"
jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 6
def run(t):
    inp = t.read_text(); st = time.perf_counter()
    p = subprocess.run(cmd, shell=True, input=inp, capture_output=True, text=True, cwd=ROOT)
    el = time.perf_counter() - st
    ref = json.loads(t.with_suffix(".ref").read_text())
    try:
        L = chk.validate(chk.parse_input(inp), p.stdout)
        sc = min(1.0, ref.get("s", 1) * ref["length"] / L) ** 2 * (0.5 + 0.5 * math.exp(-max(0, el - 1) / 5))
        info = f"L={L} ref={ref['length']}"
    except Exception as e:
        sc, info = 0.0, f"INVALID {str(e)[:40]} ref={ref['length']}"
    return t.name, sc, el, info
with ThreadPoolExecutor(jobs) as ex: res = list(ex.map(run, sorted(tdir.glob("*.in"))))
for n, sc, el, info in res: print(f"{n:>12} {sc:6.3f} {el:5.2f}s {info}")
print(f"MEAN {sum(r[1] for r in res)/len(res):.4f} n={len(res)} zero={sum(r[1]==0 for r in res)} max_time={max(r[2] for r in res):.2f}")
