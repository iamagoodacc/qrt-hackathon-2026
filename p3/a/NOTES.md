# P3 builder: black-box interface only

- HackerRank calls `vector<float> probe(float t)` and prints returned floats one per line.
- The payoff function is hidden in HackerRank's grader; the executable contains no oracle.
- Output length, allowed values, interpretation/range of t, time limit, and submission limits
  are all unspecified. Neither validity nor a positive payoff can be established locally.
- Submission feedback is noisy; final official payoff and bonus eligibility are noiseless.
- Keep the final submission as the best established candidate. Human submission is required;
  experimental probes must not displace a better known final answer without direction.

Only supplied normalization: clip(raw_noisy_score / raw_noiseless_target, 0, 1).
The underlying raw payoff formula and target are UNKNOWN; there is no defensible local score.
Input size: one float t, range unknown. Return size: unknown. Time limit: unknown.

Prepared `probe_zero.cpp`: an UNVALIDATED single-zero interface experiment, not a submission
recommendation or a proven valid baseline. It compiles in C++17 with and without LOCAL and
against the actual C++23 HackerRank hidden main. No checker.py, shared tests, fake score,
or harness best is created: no scoring rules are available to implement them honestly.
Local smoke checks exercise t=-1,0,0.5,1,100 and only establish input/output wiring.

Next required evidence: initial HackerRank sample/feedback, especially expected output count,
allowed value ranges, payoff diagnostics, and meaning of t. Asked orchestrator for this.
P3-b owns experiment design and p3/LOG.md per the statement; no edits made to their files.
Builder waiting for evidence before implementing a checker or asserting a valid baseline.
