"""Validity + time check (p4-c): python3 p4/c/vcheck.py BIN DIR [jobs]"""
import sys, subprocess, time, importlib.util
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("chk", ROOT / "p4" / "checker.py")
chk = importlib.util.module_from_spec(spec); spec.loader.exec_module(chk)
cmd, d = sys.argv[1], Path(sys.argv[2]); jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 4
def run(t):
    inp = t.read_text(); s = time.perf_counter()
    p = subprocess.run(cmd, shell=True, input=inp, capture_output=True, text=True, cwd=ROOT, timeout=30)
    el = time.perf_counter() - s
    try: L = chk.validate(chk.parse_input(inp), p.stdout); res = f"OK L={L}"
    except Exception as e: res = f"INVALID {str(e)[:50]}"
    return t.name, el, res, p.stderr.strip()[-80:]
with ThreadPoolExecutor(jobs) as ex: rs = list(ex.map(run, sorted(d.glob("*.in"))))
for n, el, res, err in rs: print(f"{n:>14} {el:5.2f}s {res} | {err}")
print("valid", sum(r[2].startswith("OK") for r in rs), "/", len(rs), "max_time", max(r[1] for r in rs))
