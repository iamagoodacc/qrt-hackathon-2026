#!/usr/bin/env python3
"""Score a solution on every test of a problem, and keep the best version.

Run a solution on all tests and score it with that problem's checker:
    python3 harness.py run p1 "python3 p1/a/solve.py"
    python3 harness.py run p1 "./p1/a/solve" --save p1/a/solve.cpp --note "skyline + SA"

With --save, the file is copied into best/<problem>/ ONLY if this run beats the
best score so far. A worse run never overwrites a better one.

See the current best for every problem:
    python3 harness.py status

Each problem folder needs:
    <problem>/checker.py   with  score(inp, out, elapsed_s) -> fraction 0..1
    <problem>/tests/*.in   test inputs (write a generator: see templates/checker_template.py)
"""
import argparse
import importlib.util
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BEST_DIR = ROOT / "best"

CONFIG = {
    "time_limit_s": 10.0,   # per-test hard limit; set with --time-limit to match the statement
    "warn_frac": 0.8,       # flag a test as SLOW above this share of the limit
    "tests_dir": "tests",
    "test_glob": "*.in",
    "stderr_tail_chars": 300,
    "min_gain": 1e-9,       # a new best must beat the old one by more than this
    "pause_file": "PAUSE",  # orchestrator: `touch PAUSE` to pause all agents, `rm PAUSE` to resume
    "plateau_min": 20,      # status flags a problem with no NEW BEST for this many minutes
}


def load_checker(problem_dir: Path):
    path = problem_dir / "checker.py"
    if not path.exists():
        sys.exit(f"No checker at {path}. Write one first (copy templates/checker_template.py).")
    spec = importlib.util.spec_from_file_location(f"checker_{problem_dir.name}", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def run_one(cmd: str, inp: str, limit: float):
    """Return (stdout or None, seconds, error message)."""
    start = time.perf_counter()
    try:
        proc = subprocess.run(cmd, shell=True, input=inp, capture_output=True,
                              text=True, timeout=limit, cwd=ROOT)
    except subprocess.TimeoutExpired:
        return None, limit, "TIMEOUT"
    elapsed = time.perf_counter() - start
    if proc.returncode != 0:
        tail = proc.stderr.strip()[-CONFIG["stderr_tail_chars"]:]
        return None, elapsed, f"CRASH exit={proc.returncode} {tail}"
    return proc.stdout, elapsed, ""


def read_best_score(problem: str) -> float:
    score_file = BEST_DIR / problem / "SCORE"
    if not score_file.exists():
        return -1.0
    return float(score_file.read_text().split()[0])


def save_if_better(problem: str, src: Path, score: float, note: str) -> None:
    folder = BEST_DIR / problem
    folder.mkdir(parents=True, exist_ok=True)
    old = read_best_score(problem)
    stamp = time.strftime("%H:%M:%S")
    with open(BEST_DIR / "log.txt", "a") as log:
        log.write(f"{stamp} {problem} {score:.4f} {src} {note}\n")
    if score <= old + CONFIG["min_gain"]:
        print(f"KEPT OLD BEST {old:.4f} (this run {score:.4f}) - nothing overwritten")
        return
    dest = folder / f"{time.strftime('%H%M%S')}_{score:.4f}_{src.name}"
    shutil.copy2(src, dest)                           # every improvement is kept
    shutil.copy2(src, folder / f"BEST{src.suffix}")   # the one to submit
    (folder / "SCORE").write_text(f"{score!r} {dest.name} {note}\n")
    print(f"NEW BEST {score:.4f} (was {max(old, 0):.4f}) -> best/{problem}/BEST{src.suffix}")


def cmd_run(args) -> None:
    problem_dir = (ROOT / args.problem).resolve()
    checker = load_checker(problem_dir)
    tests = sorted((problem_dir / CONFIG["tests_dir"]).glob(CONFIG["test_glob"]))
    if not tests:
        sys.exit(f"No tests in {problem_dir / CONFIG['tests_dir']}. Generate some first.")

    fractions, zeros, max_time = [], 0, 0.0
    for test in tests:
        inp = test.read_text()
        out, elapsed, err = run_one(args.cmd, inp, args.time_limit)
        max_time = max(max_time, elapsed)
        if out is None:
            frac, status = 0.0, err
        else:
            try:
                frac, status = float(checker.score(inp, out, elapsed)), "ok"
            except Exception as exc:  # Invalid output (or a checker bug): the grader gives 0
                frac, status = 0.0, f"INVALID: {exc}"
        frac = max(0.0, min(1.0, frac))
        zeros += frac == 0.0
        fractions.append(frac)
        slow = "  SLOW" if elapsed > CONFIG["warn_frac"] * args.time_limit else ""
        if not args.quiet or status != "ok":
            print(f"{test.name:>16}  {frac:6.3f}  {elapsed:6.2f}s  {status}{slow}")

    mean = sum(fractions) / len(fractions)
    print(f"SUMMARY {args.problem} score={mean:.4f} tests={len(tests)} "
          f"zero={zeros} max_time={max_time:.2f}s best_so_far={max(read_best_score(args.problem), 0):.4f}")
    if args.save:
        if zeros:
            print(f"NOTE: {zeros} test(s) scored 0 - check INVALID/TIMEOUT lines above")
        save_if_better(args.problem, (ROOT / args.save).resolve(), mean, args.note)
    if (ROOT / CONFIG["pause_file"]).exists():
        print(f"PAUSED: the orchestrator created {CONFIG['pause_file']}. Start nothing new: report a "
              "3-line status, then wait until the file is gone (see AGENTS.md, 'Pausing').")


def cmd_status(_args) -> None:
    if not BEST_DIR.exists():
        print("No best solutions saved yet.")
        return
    for folder in sorted(p for p in BEST_DIR.iterdir() if p.is_dir()):
        score_file = folder / "SCORE"
        if not score_file.exists():
            print(f"{folder.name:>6}  (none)")
            continue
        score, rest = score_file.read_text().strip().split(" ", 1)
        mins = (time.time() - score_file.stat().st_mtime) / 60   # SCORE is rewritten only on a NEW BEST
        flag = "  <- PLATEAU" if mins >= CONFIG["plateau_min"] else ""
        print(f"{folder.name:>6}  {float(score):.4f}  last gain {int(mins):3d} min ago{flag}  ({rest})")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)

    run = sub.add_parser("run", help="score a solution on all tests")
    run.add_argument("problem", help="problem folder, e.g. p1")
    run.add_argument("cmd", help='command that runs the solution, e.g. "python3 p1/a/solve.py"')
    run.add_argument("--save", help="solution file to keep if this run is a new best")
    run.add_argument("--note", default="", help="what you changed (goes in the log)")
    run.add_argument("--time-limit", type=float, default=CONFIG["time_limit_s"])
    run.add_argument("--quiet", action="store_true", help="only print failing tests + summary")
    run.set_defaults(func=cmd_run)

    status = sub.add_parser("status", help="show the best score per problem")
    status.set_defaults(func=cmd_status)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
