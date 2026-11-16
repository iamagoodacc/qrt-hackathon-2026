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
