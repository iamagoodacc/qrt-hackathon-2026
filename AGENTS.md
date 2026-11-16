# Contest rules for every agent (read fully before starting)

We are in a 2-hour optimisation contest on HackerRank. Each problem is scored per
hidden test on HOW GOOD the answer is (partial credit). An INVALID answer scores 0.
The person running you is the orchestrator: they assign your problem, folder and idea.

## Your name tells you your job
A brief can be just "You are agent p1-b." That means: problem `p1` (`p1/statement.md`),
working folder `p1/b/`, and role `b` below. Take the time limit from the statement.
Contest background (format, scoring, what won last time): `docs/C3_SETUP_BRIEF.md` section 1
only. The rest of that file is workspace setup, which is already done: don't run it.
If the orchestrator's brief adds an idea, it overrides the default idea for your role.

- **a - Builder (support role, usually Codex).** You own the scoreboard. Do workflow steps 1-3
  FAST (aim for 20 minutes): checker, tests, then the simplest VALID solution, saved with the
  harness. Tell the orchestrator the moment the checker and tests are ready, then STOP and wait:
  don't start optimising (this role runs on limited credits). Later jobs the orchestrator may
  give you: fix the checker when HackerRank disagrees with it, or add harder tests.
- **b - Main optimiser (usually Claude).** Simulated annealing in C++, starting from
  `templates/anneal.cpp`. Begin from a good greedy answer, use moves that keep the answer valid
  (or penalise invalid states heavily), and tune the CONFIG temperatures by experiment.
- **c - Second optimiser (optional, usually Claude).** First audit `pN/checker.py` against the
  statement line by line and report any mismatch to the orchestrator (don't edit it yourself).
  Then try an approach b is NOT using, e.g. beam search, exact search for small inputs, large
  neighbourhood search (destroy part of the answer and rebuild it), or many restarts of a
  randomised greedy.
- **Not agent a?** Never create or edit `pN/checker.py` or `pN/tests/`. If they don't exist yet,
  write your NOTES.md and plan, then poll with `sleep 60; ls pN/tests | head -3` until they do.
- Read `best/pN/SCORE` for the current best. You may read other agents' folders for ideas,
  but only write in your own.

Build and score a C++ solution (save the .cpp source, not the binary):
`g++ -std=c++17 -O2 -o pN/<you>/solve pN/<you>/solve.cpp && python3 harness.py run pN "./pN/<you>/solve" --save pN/<you>/solve.cpp --note "<idea>" --time-limit <T>`

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
   If the harness printed NEW BEST, start your line with `NEW BEST` so the orchestrator submits it.
   Also append that line to `pN/<you>/NOTES.md`, so a fresh agent can carry on where you stopped.
   If NOTES.md already has a log when you start, you ARE that fresh agent: read it and continue.
5. Keep going until told to stop. If an idea fails twice, say so and propose the next one.

## Hard rules for the solution you submit
- One self-contained file. Read stdin, write the answer to stdout. Debug output ONLY to stderr.
- Language: C++17 for the submitted solution (team decision). Python is fine for checkers,
  generators and offline helper scripts.
- Standard library only (Python 3 or C++17). Assume numpy, OR-Tools etc. are NOT available on
  HackerRank unless the orchestrator says otherwise. (You MAY use them in offline helper scripts.)
- Respect the time limit: loop until a time budget set in a CONFIG block (about 80% of the
  limit), then print the best answer found so far. Never risk a timeout.
- C++: must compile with `g++ -std=c++17 -O2 -o solve solve.cpp`.
- Verify validity with the checker before every save. Never save an answer that scores 0 anywhere
  unless the orchestrator agrees.
- No magic numbers buried in code: tunable values go in a CONFIG block at the top.
