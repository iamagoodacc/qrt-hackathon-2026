# C3 Hackathon Workspace — Setup Brief for Claude Code

**Read this whole file before doing anything.** It holds the context, the target setup, the ordered
tasks with checks, and (in Appendix A) the exact contents of every file to create.

- Owner: Zihan (University of Edinburgh), competing with a team of 3.
- Deadline: everything must work before **18:00 today, Monday 16 November 2026**. Work quickly,
  keep it simple, and don't add features nobody asked for.

---

## 1. What you are helping with

**Event:** QRT x CompSoc "C3 Hackathon: Code, Compute, Compete", Edinburgh, 16 Nov 2026,
18:00–21:00, Informatics Forum G.07. QRT (Qube Research & Technologies) designed the logic and
coding challenges with help from an LLM. Teams of up to 4. Every participant gets $25 of OpenAI
API credits. The top teams win tech prizes and qualify for the Grand Final in London.

**Expected format** (based on the identical QRT round at Imperial on 26 Oct 2026; the problems
will be new):
- A 2-hour timed test on **HackerRank**: four optimisation problems plus one bonus.
- **Partial credit:** each hidden test pays a share of its points based on answer *quality*
  (e.g. a smaller box or a shorter loop). An **invalid answer scores 0**. Sample tests are not scored.
- The bonus only counts if the team reaches **70% on every main problem**.
- The Imperial problems were square packing (score = (L/A)^20, very harsh), word-search grid
  generation, a noisy black-box function, Masyu (shortest loop, with a runtime penalty after 1 s and
  a 10 s timeout), and a bonus that reassembled shredded text.
- Imperial results: the winner scored 3988/4300 (93%); the average team scored 2301/4300 (54%).

**How the winner did it** (this is our strategy):
- One person **orchestrated many AI coding agents** in parallel (15 Claude Code and 7 Codex
  sessions). He wrote little code by hand.
- He built a **fast feedback loop** first: a runner that scores any solution on all tests.
- Each agent followed a fixed protocol: restate the problem, verify answers, measure every test,
  and only replace the saved best if the score went up.
- Several independent attempts at a problem, keeping the best, beat one bigger attempt.
- The usual techniques were greedy baselines, then simulated annealing, constraint search and
  large neighbourhood search. Hot loops went in C++ (`g++ -O2`).

**Our plan:** each teammate orchestrates 2–3 agents (Codex CLI + Claude Code CLI) inside tmux.
A shared repo `c3` holds the rules file (`AGENTS.md`), a scoring harness (`harness.py`), a
launcher (`agents.sh`), templates, and a practice problem (`p0`).

**Open rules questions** (the user will ask the organisers on the night; do NOT build tooling for
them): Is outside compute such as DICE allowed? Is hard-coding answers for hidden tests allowed?
Does one person submit for the team?

---

## 2. Environment (important)

- **Laptop: Windows with WSL Ubuntu.** All tools and files live **inside Ubuntu**, not on a Windows drive.
- **Repo location: `~/c3`** in the Linux home folder. **Never** use `/mnt/c`, `/mnt/e` or any `/mnt/*`
  path: Linux tools are much slower there, and `chmod`/line endings break.
- VS Code must be **connected to WSL** (bottom-left corner shows "WSL: Ubuntu"; open it with
  `code ~/c3` from an Ubuntu terminal). If your working directory starts with `/mnt/`, **stop** and
  tell the user to reopen VS Code that way.
- Tools needed in Ubuntu: `python3`, `g++`, `git`, `tmux`, `node` (18+), `codex` (OpenAI Codex CLI) and
  `claude` (Claude Code CLI). The contest code itself uses only the Python/C++ standard library.
- Optional later: DICE compute (`student.compute.inf.ed.ac.uk` through the `student.ssh.inf.ed.ac.uk`
  gateway, VPN needed). **Not part of this setup.**

---

## 3. Target layout

```
~/c3/
├── AGENTS.md                 rules every agent follows (Codex reads it directly)
├── CLAUDE.md                 one line: @AGENTS.md (so Claude Code reads the same rules)
├── harness.py                scores a solution on all tests; keeps the best version
├── agents.sh                 opens one tmux window per agent
├── .gitignore
├── templates/
│   ├── checker_template.py
│   ├── anneal.py
│   └── anneal.cpp
├── p0/                       practice problem (square packing)
│   ├── statement.md
│   ├── checker.py            checker + test generator
│   └── baseline.py           weak but valid baseline (scores 0.0456)
└── p1/ p2/ p3/ p4/           empty, each with a .gitkeep (statements get pasted in on the night)
```

---

## 4. Tasks — do them in order and check each one

**T1. Check the environment.** Run `pwd` and confirm you are in `~/c3` (create it if missing). Make
sure the path does NOT start with `/mnt/`. Then run:
`python3 --version; g++ --version; git --version; tmux -V; node --version`.
Report anything missing.

**T2. Install missing system tools.** `sudo` needs the user's password, so don't run it yourself.
**Print this command for the user to run** in their own Ubuntu terminal:
`sudo apt update && sudo apt install -y python3 python3-pip g++ git tmux nodejs npm`

**T3. Create every file in Appendix A exactly as written**, byte for byte, at the paths shown. Don't
"improve", reformat or shorten them. Also create:
- `.gitignore` containing:
  ```
  __pycache__/
  *.pyc
  .env
  *.o
  solve
  a.out
  p*/*/solve
  ```
- An empty `p1/.gitkeep`, `p2/.gitkeep`, `p3/.gitkeep` and `p4/.gitkeep`.

**T4. Prepare and verify.**
```bash
sed -i 's/\r$//' agents.sh harness.py   # remove any Windows line endings
chmod +x agents.sh harness.py
python3 p0/checker.py gen               # expect: wrote 20 tests to .../p0/tests
python3 harness.py run p0 "python3 p0/baseline.py" --time-limit 2 --save p0/baseline.py --note baseline
python3 harness.py run p0 "python3 p0/baseline.py" --time-limit 2 --save p0/baseline.py --quiet
python3 harness.py status
g++ -std=c++17 -O2 -o /tmp/anneal_check templates/anneal.cpp && echo "anneal.cpp compiles"
```
Expected output:
- The first run ends with `SUMMARY p0 score=0.0456 tests=20 zero=0 ...` and `NEW BEST 0.0456`.
- The second run prints `KEPT OLD BEST 0.0456 ... nothing overwritten`.
- `status` shows the p0 line.

If the score differs from 0.0456, a file was copied wrong. Recheck it against Appendix A. Then reset
with `rm -rf best` and run T4 again so `best/` starts clean.

**T5. Git.** Run `git init` (if it's not already a repo), then `git add -A && git commit -m "C3 starter kit"`.
Ask the user whether a team GitHub repo exists. If they give a URL, run `git remote add origin <url>`
and push. Don't commit the generated `best/` folder from the T4 test; delete it first (`rm -rf best`).

**T6. Agent CLIs** (check each; install whatever is missing):
- **Codex:** `codex --version`. If it's missing, install it without sudo:
  `curl -fsSL https://chatgpt.com/codex/install.sh | sh`. The npm fallback is
  `npm config set prefix ~/.npm-global && npm install -g @openai/codex`, then add `~/.npm-global/bin` to PATH in `~/.bashrc`.
- **Claude Code CLI inside WSL:** `claude --version`. If it's missing: `curl -fsSL https://claude.ai/install.sh | bash`.
- **USER STEP: never ask the user to paste an API key into chat, and never write a key into a file.**
  Tell the user to run this themselves in their own terminal:
  ```bash
  export OPENAI_API_KEY="sk-..."      # key from platform.openai.com/api-keys
  printenv OPENAI_API_KEY | codex login --with-api-key
  codex login status
  ```
  They should also run `claude` once to log in.

**T7. Smoke-test the launcher without starting real agents:**
```bash
tmux new-session -d -s base
SOCK=$(tmux display -p '#{socket_path}')
TMUX="$SOCK,1,0" CODEX_CMD="sleep 30" CLAUDE_CMD="sleep 30" SESSION=c3test ./agents.sh p0-a:codex p0-b:claude
tmux list-windows -t c3test        # expect windows: home, p0-a, p0-b
tmux kill-server
```

**T8. Report back** with a short checklist: what passed, and what the user still has to do (the T2
sudo command if needed, API key login, `claude` login). Then point them to Section 5 for the
practice run.

---

## 5. How the workspace is used (so you can answer questions about it later)

- **Roles:** the user is the *orchestrator*. Each agent gets one problem, one folder (`pN/<letter>/`) and
  one idea. All agents start in the repo root so they load `AGENTS.md`.
- **Launch:** `./agents.sh p1-a:codex p1-b:claude` opens one tmux window per agent.
  The tmux prefix is `Ctrl-b`: then `w` lists windows, `n`/`p` goes to the next/previous one, and `d` detaches.
- **Codex** runs as `codex --sandbox workspace-write` (edits and commands stay inside the repo).
  Answer "yes" when it asks to trust the folder, or AGENTS.md won't load. The default model is `gpt-6.1-sol`;
  for the hardest problem use `-m gpt-6-astra`.
- **Scoreboard:** `python3 harness.py run pN "<cmd>" --save <file> --note "<idea>" --time-limit <T>`
  scores every test with `pN/checker.py`, and copies the file to `best/pN/BEST.*` **only if** the score improved.
  `python3 harness.py status` shows the best score per problem.
- **Check-in rhythm:** every ~5 min, read each agent's last score line and `harness.py status`. Re-brief
  an agent after 15 min without improvement. Submit `best/pN/BEST.*` to HackerRank and compare with the
  local score (a big gap means the checker or tests are wrong).
- **Brief for the first agent on a problem:**
  ```
  You are agent p1-a. Problem: p1/statement.md. Your folder: p1/a/.
  Follow AGENTS.md. First build p1/checker.py from templates/checker_template.py with every rule
  and the EXACT scoring formula, test it on the sample, then generate 20+ tests incl. max sizes.
  Then write the simplest valid solution and save it with harness.py --save.
  Time limit is <T> seconds. Report one score line after every run.
  ```
- **Brief for a second agent (a different idea):**
  ```
  You are agent p1-b. Problem: p1/statement.md. Your folder: p1/b/. Follow AGENTS.md.
  Use the existing p1/checker.py and p1/tests (don't edit them).
  Approach: <e.g. C++ simulated annealing from templates/anneal.cpp, moves = swap two items>.
  Beat best/p1/SCORE. Save with harness.py --save. Report one score line after every run.
  ```
- **Practice run (45 min, on p0):** two agents improve on `p0/baseline.py` (one tries widths plus a
  skyline packer, the other C++ annealing). Goal: get from 0.046 to above 0.5 and get comfortable with the routine.
- **On the night:** in the first 10 min, read every problem. Then one problem per teammate. Get a valid
  submission on every problem by about 40 min, then 70%+ on each (bonus gate), then improve. In the
  last 10 min, stop experimenting and confirm HackerRank holds every `BEST`.

## 6. Working style for this user

- Explain things in plain English, with minimal assumed maths.
- Tunable values go in a `CONFIG` block, never as magic numbers.
- When changing existing files, make minimal, targeted edits rather than rewriting whole files.

---

## Appendix A — exact file contents

Each block below is the full content of one file. Create the files exactly.

### `AGENTS.md`

````markdown
# Contest rules for every agent (read fully before starting)

We are in a 2-hour optimisation contest on HackerRank. Each problem is scored per
hidden test on HOW GOOD the answer is (partial credit). An INVALID answer scores 0.
The person running you is the orchestrator: they assign your problem, folder and idea.

## Folder layout
- `harness.py`        - scores a solution on all tests and keeps the best version. Never edit it.
- `templates/`        - checker + simulated-annealing skeletons (Python and C++).
- `pN/statement.md`   - the problem statement (pasted by the orchestrator).
- `pN/checker.py`     - shared checker for problem N. `score(inp, out, elapsed_s)` returns 0..1.
- `pN/gen.py`, `pN/tests/*.in` - test generator and generated tests.
- `pN/<you>/`         - YOUR working folder. Only write inside it (plus checker/gen if told to).
- `best/`             - written ONLY by `harness.py --save`. Never edit by hand.

## Workflow (in this order)
1. Read `pN/statement.md`. Write `pN/<you>/NOTES.md` with: the rules in 3-6 bullets,
   the exact scoring formula, the input size limits and the time limit.
2. If you were told to build the checker (or `pN/checker.py` is missing and you are agent `a`):
   copy `templates/checker_template.py` to `pN/checker.py` and implement EVERY rule and the
   EXACT scoring formula. Test it on the statement's sample. Then write `pN/gen.py` that writes
   20+ tests to `pN/tests/` covering small, medium and the maximum sizes from the constraints.
3. Write the SIMPLEST valid solution first (greedy is fine). Score and save it:
   `python3 harness.py run pN "<command>" --save pN/<you>/<file> --note "baseline" --time-limit <T>`
4. Improve ONE idea at a time. After each change, run the harness on ALL tests and report
   one line: `score=0.812 (best 0.790) max_time=3.1s idea=<what changed>`.
5. Keep going until told to stop. If an idea fails twice, say so and propose the next one.

## Hard rules for the solution you submit
- One self-contained file. Read stdin, write the answer to stdout. Debug output ONLY to stderr.
- Standard library only (Python 3 or C++17). Assume numpy, OR-Tools etc. are NOT available on
  HackerRank unless the orchestrator says otherwise. (You MAY use them in offline helper scripts.)
- Respect the time limit: loop until a time budget set in a CONFIG block (about 80% of the
  limit), then print the best answer found so far. Never risk a timeout.
- C++: must compile with `g++ -std=c++17 -O2 -o solve solve.cpp`.
- Verify validity with the checker before every save. Never save an answer that scores 0 anywhere
  unless the orchestrator agrees.
- No magic numbers buried in code: tunable values go in a CONFIG block at the top.
````

### `CLAUDE.md`

````markdown
@AGENTS.md
````

### `harness.py`

````python
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


def cmd_status(_args) -> None:
    if not BEST_DIR.exists():
        print("No best solutions saved yet.")
        return
    for folder in sorted(p for p in BEST_DIR.iterdir() if p.is_dir()):
        score_file = folder / "SCORE"
        line = score_file.read_text().strip() if score_file.exists() else "(none)"
        print(f"{folder.name:>6}  {line}")


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
````

### `agents.sh`

````bash
#!/usr/bin/env bash
# Launch several coding agents, one tmux window each, all started from the repo root
# (so every agent sees AGENTS.md / CLAUDE.md and can run harness.py).
#
# Usage:   ./agents.sh p1-a:codex p1-b:claude p2-a:codex
#          (name:tool  - the name labels the window; tell the agent its folder in your brief)
#
# tmux keys (press Ctrl-b, let go, then the key):
#   n / p   next / previous window        w   pick a window from a list
#   0-9     jump to window number          d   detach (agents keep running; reattach: tmux attach -t c3)
set -euo pipefail

SESSION="${SESSION:-c3}"
ROOT="$(cd "$(dirname "$0")" && pwd)"
CODEX_CMD="${CODEX_CMD:-codex --sandbox workspace-write}"
CLAUDE_CMD="${CLAUDE_CMD:-claude}"

if [ "$#" -eq 0 ]; then
  sed -n '2,12p' "$0"
  exit 1
fi

if ! tmux has-session -t "$SESSION" 2>/dev/null; then
  tmux new-session -d -s "$SESSION" -n home -c "$ROOT"
fi

for spec in "$@"; do
  name="${spec%%:*}"
  tool="${spec##*:}"
  case "$tool" in
    codex)  cmd="$CODEX_CMD" ;;
    claude) cmd="$CLAUDE_CMD" ;;
    *) echo "unknown tool '$tool' in '$spec' (use codex or claude)"; exit 1 ;;
  esac
  # "; exec bash" keeps the window open if the agent exits, so you can read what happened
  tmux new-window -t "$SESSION" -n "$name" -c "$ROOT" "$cmd; exec bash"
done

if [ -n "${TMUX:-}" ]; then
  echo "Added windows to session '$SESSION'. Switch with Ctrl-b w."
else
  tmux attach -t "$SESSION"
fi
````

### `templates/checker_template.py`

````python
"""Checker template. Copy to pN/checker.py and fill in the three TODOs.

score() must return the FRACTION (0..1) of one test's points, computed EXACTLY like
the statement's formula. Raise Invalid for ANY rule break: the real grader gives 0.

Running this file directly generates tests:  python3 pN/checker.py gen
(Put the generator here or in pN/gen.py, whichever you prefer.)
"""
import random
import sys
from pathlib import Path

CONFIG = {
    "num_tests": 20,
    "seed": 1,
    # TODO: size limits copied from the statement, e.g. "max_n": 100
}


class Invalid(Exception):
    pass


def parse_input(inp: str):
    tokens = inp.split()
    # TODO: turn tokens into the problem's data
    return tokens


def parse_output(out: str, data):
    lines = out.strip().splitlines()
    # TODO: parse; raise Invalid("line 3: expected 2 integers") on any format problem
    return lines


def score(inp: str, out: str, elapsed_s: float = 0.0) -> float:
    data = parse_input(inp)
    answer = parse_output(out, data)
    # TODO 1: check EVERY rule; raise Invalid("which rule failed, where") on the first break
    # TODO 2: compute the statement's scoring formula (include any runtime factor via elapsed_s)
    return 0.0


def generate_tests(folder: Path) -> None:
    rng = random.Random(CONFIG["seed"])
    folder.mkdir(exist_ok=True)
    for i in range(CONFIG["num_tests"]):
        # TODO 3: build an input; cover small, medium and the MAXIMUM sizes
        text = "TODO\n"
        (folder / f"{i:02d}.in").write_text(text)
    print(f"wrote {CONFIG['num_tests']} tests to {folder}")


if __name__ == "__main__" and sys.argv[1:] == ["gen"]:
    generate_tests(Path(__file__).resolve().parent / "tests")
````

### `templates/anneal.py`

````python
"""Simulated annealing skeleton (Python). Fill in initial(), score() and tweak().

Idea in one line: keep making small random changes; always keep improvements, and
early on ("hot") sometimes keep a worse one so you don't get stuck; get pickier over time.
Higher score() = better. Prints the best answer found before the time budget runs out.
"""
import math
import random
import sys
import time

CONFIG = {
    "time_budget_s": 8.0,  # about 80% of the problem's time limit
    "start_temp": 1.0,     # roughly the size of a typical score change early on
    "end_temp": 0.01,      # much smaller: only near-improvements accepted at the end
    "seed": 1,
}


def initial(data):
    """A simple VALID answer (greedy is fine)."""
    raise NotImplementedError


def score(data, state) -> float:
    """Higher is better. Must match the real scoring as closely as possible."""
    raise NotImplementedError


def tweak(data, state):
    """Return a NEW state with one small random change (move/swap one thing)."""
    raise NotImplementedError


def anneal(data):
    rng = random.Random(CONFIG["seed"])
    random.seed(CONFIG["seed"])
    start = time.perf_counter()
    cur = best = initial(data)
    cur_s = best_s = score(data, cur)
    iters = 0
    while (elapsed := time.perf_counter() - start) < CONFIG["time_budget_s"]:
        frac = elapsed / CONFIG["time_budget_s"]
        temp = CONFIG["start_temp"] * (CONFIG["end_temp"] / CONFIG["start_temp"]) ** frac
        cand = tweak(data, cur)
        cand_s = score(data, cand)
        if cand_s >= cur_s or rng.random() < math.exp((cand_s - cur_s) / temp):
            cur, cur_s = cand, cand_s
            if cur_s > best_s:
                best, best_s = cur, cur_s
        iters += 1
    print(f"iters={iters} best={best_s:.4f}", file=sys.stderr)
    return best


def main():
    data = sys.stdin.read().split()  # replace with real parsing
    best = anneal(data)
    print(best)  # replace with the required output format


if __name__ == "__main__":
    main()
````

### `templates/anneal.cpp`

````cpp
// Simulated annealing skeleton (C++). Fill in State, initial(), score(), tweak().
// Build: g++ -std=c++17 -O2 -o solve solve.cpp
// Same idea as anneal.py, typically 10-100x more iterations in the same time.
// Speed-up for later: change the state in place and undo rejected moves instead of copying.
#include <bits/stdc++.h>
using namespace std;

struct Config {
    double time_budget_s = 8.0;  // about 80% of the problem's time limit
    double start_temp = 1.0;     // roughly the size of a typical score change early on
    double end_temp = 0.01;
    uint64_t seed = 1;
    int clock_check_every = 256; // reading the clock is slow; check it every N iterations
};

struct Input { /* TODO: problem data */ };
struct State { /* TODO: your answer */ };

mt19937_64 rng;

State initial(const Input& in) { return State{}; }             // TODO: simple VALID answer
double score(const Input& in, const State& s) { return 0.0; }  // TODO: higher is better
State tweak(const Input& in, const State& s) { return s; }     // TODO: one small random change

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    const Config cfg;
    rng.seed(cfg.seed);

    Input in;  // TODO: read stdin into `in`

    const auto t0 = chrono::steady_clock::now();
    auto elapsed = [&] { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); };
    uniform_real_distribution<double> unit(0.0, 1.0);

    State cur = initial(in), best = cur;
    double cur_s = score(in, cur), best_s = cur_s;
    long long iters = 0;
    double t = 0.0;
    while (true) {
        if (iters % cfg.clock_check_every == 0 && (t = elapsed()) >= cfg.time_budget_s) break;
        const double frac = t / cfg.time_budget_s;
        const double temp = cfg.start_temp * pow(cfg.end_temp / cfg.start_temp, frac);
        State cand = tweak(in, cur);
        const double cand_s = score(in, cand);
        if (cand_s >= cur_s || unit(rng) < exp((cand_s - cur_s) / temp)) {
            cur = std::move(cand);
            cur_s = cand_s;
            if (cur_s > best_s) { best = cur; best_s = cur_s; }
        }
        ++iters;
    }
    cerr << "iters=" << iters << " best=" << best_s << "\n";
    // TODO: print `best` in the required output format to stdout
    return 0;
}
````

### `p0/statement.md`

````markdown
# P0 (practice): Square packing

Based on Q1 of the QRT x Imperial round (26 Oct 2026), restated for practice.

## Task
You are given n squares with integer side lengths. Place every square on an integer grid,
axis-aligned, with no two squares overlapping (touching edges is fine). Make the
bounding rectangle that encloses all squares as SMALL IN AREA as possible.

## Input
First line: n. Then n lines, one side length each.
Limits (practice): 1 <= n <= 100, 1 <= side <= 30.

Sample:
```
3
1
4
5
```

## Output
n lines `X Y` = top-left corner of each square, in input order.
X increases downward, Y increases to the right. Coordinates are integers (may be any value).

## Scoring (per test)
Let m = largest side, S = sum of squared sides,
L = min over integers W >= m of  W * max(m, ceil(S / W))   (a lower bound on the area),
A = area of your bounding rectangle.

Points = (L / A) ^ 20. Invalid output (overlap, wrong line count, non-integers) = 0.
Note how harsh the power 20 is: 5% above the bound scores about 0.38.

## Time limit
2 seconds per test (practice setting).
````

### `p0/checker.py`

````python
"""Checker + test generator for P0 (square packing practice problem).

Score a solution:   python3 harness.py run p0 "python3 p0/baseline.py" --time-limit 2
Generate tests:     python3 p0/checker.py gen
"""
import math
import random
import sys
from pathlib import Path

CONFIG = {
    "num_tests": 20,
    "seed": 1,
    "min_n": 5,
    "max_n": 100,
    "max_side": 30,
    "power": 20,
}


class Invalid(Exception):
    pass


def parse_input(inp: str):
    tokens = inp.split()
    n = int(tokens[0])
    sides = [int(t) for t in tokens[1:1 + n]]
    if len(sides) != n:
        raise ValueError("input has fewer sides than n")
    return sides


def parse_output(out: str, n: int):
    lines = [ln for ln in out.strip().splitlines() if ln.strip()]
    if len(lines) != n:
        raise Invalid(f"expected {n} lines, got {len(lines)}")
    coords = []
    for i, ln in enumerate(lines, 1):
        parts = ln.split()
        if len(parts) != 2:
            raise Invalid(f"line {i}: expected 2 integers, got {ln!r}")
        try:
            coords.append((int(parts[0]), int(parts[1])))
        except ValueError:
            raise Invalid(f"line {i}: not integers: {ln!r}")
    return coords


def lower_bound(sides) -> int:
    m = max(sides)
    total = sum(s * s for s in sides)
    best = None
    for width in range(m, max(m, math.ceil(total / m)) + 1):
        area = width * max(m, math.ceil(total / width))
        best = area if best is None else min(best, area)
    return best


def score(inp: str, out: str, elapsed_s: float = 0.0) -> float:
    sides = parse_input(inp)
    coords = parse_output(out, len(sides))
    rects = [(x, y, x + s, y + s) for (x, y), s in zip(coords, sides)]
    for i in range(len(rects)):
        ax0, ay0, ax1, ay1 = rects[i]
        for j in range(i + 1, len(rects)):
            bx0, by0, bx1, by1 = rects[j]
            if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1:
                raise Invalid(f"squares {i + 1} and {j + 1} overlap")
    height = max(r[2] for r in rects) - min(r[0] for r in rects)
    width = max(r[3] for r in rects) - min(r[1] for r in rects)
    area = height * width
    return (lower_bound(sides) / area) ** CONFIG["power"]


def generate_tests(folder: Path) -> None:
    rng = random.Random(CONFIG["seed"])
    folder.mkdir(exist_ok=True)
    for i in range(CONFIG["num_tests"]):
        # sizes grow with the test index, the last ones at the maximum
        frac = i / (CONFIG["num_tests"] - 1)
        n = round(CONFIG["min_n"] + frac * (CONFIG["max_n"] - CONFIG["min_n"]))
        max_side = rng.randint(5, CONFIG["max_side"])
        sides = [rng.randint(1, max_side) for _ in range(n)]
        (folder / f"{i:02d}.in").write_text(f"{n}\n" + "\n".join(map(str, sides)) + "\n")
    print(f"wrote {CONFIG['num_tests']} tests to {folder}")


if __name__ == "__main__" and sys.argv[1:] == ["gen"]:
    generate_tests(Path(__file__).resolve().parent / "tests")
````

### `p0/baseline.py`

````python
"""P0 baseline: shelf packing. Valid but weak - the point of the mock is to beat it.

Sort squares biggest first, fill rows left to right up to a fixed width, start a new
row when the next square doesn't fit. Row height = the first (largest) square in it.
"""
import math
import sys

CONFIG = {
    "width_factor": 1.2,  # target width = this * sqrt(total area)
}


def main():
    tokens = sys.stdin.read().split()
    n = int(tokens[0])
    sides = [int(t) for t in tokens[1:1 + n]]
    total = sum(s * s for s in sides)
    width = max(max(sides), math.ceil(CONFIG["width_factor"] * math.sqrt(total)))

    order = sorted(range(n), key=lambda i: -sides[i])
    pos = [None] * n
    row_top, row_height, col = 0, 0, 0
    for i in order:
        s = sides[i]
        if col + s > width:          # start a new row
            row_top += row_height
            row_height, col = 0, 0
        pos[i] = (row_top, col)      # X = down, Y = right
        col += s
        row_height = max(row_height, s)

    print("\n".join(f"{x} {y}" for x, y in pos))


if __name__ == "__main__":
    main()
````

---

## Sources (context, for reference)

- 1st place write-up, QRT x Imperial "Code. Compute. Compete" (26 Oct 2026): https://github.com/xRyann2255/qrt-hackathon-2026
- Imperial problem statements (transcribed): https://github.com/xRyann2255/qrt-hackathon-2026/blob/main/QUESTIONS.md
- Edinburgh event listing: https://hackathonhub.eu/events/qrt-x-compsoc-c3-coding-hackathon-edinburgh-2026
- Codex CLI guide: https://blakecrosley.com/guides/codex
- Claude Code setup: https://code.claude.com/docs/en/setup
- WSL file-system advice: https://learn.microsoft.com/en-us/windows/wsl/filesystems
- Informatics compute servers: https://computing.help.inf.ed.ac.uk/compute-servers
