# Team plan - QRT x CompSoc C3 Hackathon

Monday 16 Nov 2026, 18:00-21:00, Informatics Forum G.07. 2-hour HackerRank test:
4 optimisation problems + 1 bonus. Partial credit per test for HOW GOOD the answer is;
an invalid answer scores 0. The bonus only counts if we reach 70% on EVERY main problem.

Our approach (what won the last round): each of us runs 2-3 AI coding agents in parallel.
The AI writes the code; we read problems, steer the agents, and submit the best answers.

## Before 18:00 - each of us
1. **Laptop:** Linux, Mac, or Windows with WSL Ubuntu. Needs `python3`, `g++`, `git`, `tmux`,
   `node` 18+, and the `codex` CLI (plus the `claude` CLI if you have Claude). Codex runs on
   the free $25 OpenAI credit we get at the event.
   - Mac: Apple's `g++` is really clang and can't compile `bits/stdc++.h`. Run
     `brew install gcc tmux` and tell your agent to use the Homebrew g++ (e.g. `g++-14`).
2. **Get the repo** into `~/c3` (Zihan will send the link or a zip). On Windows, put it in the
   Ubuntu home folder, not on the C: drive.
3. **Check your setup the easy way:** open Claude Code or Codex in `~/c3` and say:
   "Follow docs/C3_SETUP_BRIEF.md tasks T1, T2, T4, T6 and T7 to check this machine.
   The files already exist, so skip T3 and T5."
4. **Read `ORCHESTRATOR.md`** (5 min): how to launch agents, tmux, and submitting.
5. If you have 45 minutes, do the practice run on `p0` (section 8 of ORCHESTRATOR.md).

## On the night - who does what
- **0-10 min:** all three of us read ALL problems, then split them:
  - one problem each (p1, p2, p3 by default; swap if someone prefers a problem),
  - whoever's problem looks easiest also takes p4 and launches agents on it straight away.
- **Each owner, on their own laptop:** paste the statement into `pN/statement.md`, then run
  `./agents.sh pN-a:claude pN-b:codex` and type `You are agent pN-a.` / `You are agent pN-b.`
  (no Claude? use `pN-a:codex` instead).
- **Post in the group chat** when your problem: has its first valid submission, passes 70%,
  or scores very differently on HackerRank than locally.
- **Targets:** valid submission on every problem by ~40 min; 70%+ on all four next (unlocks
  the bonus); then improve the weakest. Last 10 min: no experiments, just confirm every best
  answer is submitted.
- **Bonus:** only once all four problems are 70%+. Whoever finishes first takes it (use `p5`).

## Questions for the organisers (ask at 18:00)
1. Is outside compute (e.g. DICE servers) allowed?
2. Is hard-coding answers for the hidden tests allowed?
3. Does one person submit for the whole team, or can each of us submit?
   If one person submits: owners send their `best/pN/BEST.cpp` to that person in the group chat.
