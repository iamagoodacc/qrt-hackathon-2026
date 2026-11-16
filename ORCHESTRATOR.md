# Orchestrator guide (for humans)

Your job is to be the manager, not the coder: start agents, tell each one its name, keep
them unblocked, check the scoreboard, and paste the best answers into HackerRank.

## 1. Who does what

Each problem gets 2 or 3 agents. Their name tells them their job (the rules are in
`AGENTS.md`, which they read automatically):

| Window | Tool   | Role      | What it does |
|--------|--------|-----------|--------------|
| `p1-a` | claude | Builder   | Reads the problem, builds the local scorer (`checker.py`) and tests, writes the first valid answer, then improves it |
| `p1-b` | codex  | Annealer  | C++ "simulated annealing": keeps making small random changes and keeping the good ones |
| `p1-c` | codex  | Wildcard  | Optional. Double-checks the scorer against the statement, then tries a completely different method |

**The brief is one line.** In each agent's window type, e.g.:
```
You are agent p1-a.
```
To steer an agent, add an idea: `You are agent p1-b. Idea: moves = swap two items.`

Why two AI tools: different AIs try different ideas, and Codex runs on the free $25 OpenAI credit.

## 2. tmux in 2 minutes

tmux = one terminal holding many windows, one per agent.

- **Use the Ubuntu app (Windows Terminal), not VS Code's terminal.** VS Code steals `Ctrl-b`
  (it toggles its sidebar). Keep VS Code open beside it for reading files.
- **Start agents:** `cd ~/c3 && ./agents.sh p1-a:claude p1-b:codex`
- **Switch windows:** click a window name in the bar at the bottom of the screen.
- **Scroll:** mouse wheel. **Copy text:** hold Shift while you drag to select.
- **Keys** (press `Ctrl-b`, let go, then the key):
  - `w` - list all windows, pick one with the arrow keys + Enter
  - `n` / `p` - next / previous window
  - `d` - detach: hides tmux, agents keep running. Come back with `tmux attach -t c3`
- **Add agents later:** go to the `home` window and run `./agents.sh p2-a:claude` again.
- **Agent window shows a plain `$` prompt?** The agent quit. Type `claude` or
  `codex --sandbox workspace-write` to restart it, then re-brief it.
- **Close everything:** `tmux kill-session -t c3`
- **tmux annoying you?** Just open one terminal tab per agent and run `cd ~/c3 && claude`
  (or `codex --sandbox workspace-write`) in each. It works the same.

## 3. First launch of each tool

- **Codex** asks whether to trust the folder: say **yes**, or it won't read `AGENTS.md`.
- **Claude** asks before running commands. Choose "Yes, and don't ask again" for routine
  commands (`g++`, `python3 harness.py`) so it stops asking. `Shift+Tab` switches it into
  "accept edits" mode so file edits stop asking too.
- A "stuck" agent is very often just waiting for you to answer a question. Check every window.

## 4. Your loop on the night

**Minutes 0-10:** read every problem with your team and agree who owns what (see `TEAM.md`).
For each problem you own, copy the WHOLE statement from HackerRank (story, input/output
format, constraints, sample, scoring formula, time limit) into `pN/statement.md`.
Then launch and brief the agents.

**Every ~5 minutes:**
1. Click through each agent window. Answer any question it is waiting on.
2. In the `home` window: `python3 harness.py status` (best score per problem).
3. Score went up? Submit it (section 6).

**Agent not improving for 15 minutes?** Give it a new idea (list below). If it seems
confused, start it fresh: type `/clear` (Claude) or `/new` (Codex), then the one-line brief again.

Ideas to hand out when stuck:
- "Start the annealing from the current best answer instead of from scratch."
- "Try a different kind of move" (swap two things / move one thing / reverse a section).
- "Make each step faster: change the answer in place and undo rejected moves."
- "Restart from several different random starting answers and keep the best."
- "Tune the CONFIG temperatures: try start_temp 10x bigger and 10x smaller."
- "Look at which tests score lowest and focus on those."

## 5. Check the checker (the most important thing you do)

The agents chase the score that `checker.py` gives. If the checker is wrong, they chase the
wrong thing. As soon as the first valid answer exists, submit it to HackerRank and compare:
- HackerRank's score should be roughly the harness score (not exact: their tests differ).
- HackerRank says wrong answer / 0 / much lower? Tell agent `a`:
  `HackerRank gave <X>, the harness gave <Y>. Re-read the statement and find the rule the checker misses.`

## 6. Submitting

The file to submit is always `best/pN/BEST.cpp` (the harness only overwrites it when the score improves).
1. Copy it to your clipboard: `clip.exe < best/p1/BEST.cpp`
2. HackerRank: open the problem, set the language to C++, paste over the editor, Submit.
3. Compare the score with `python3 harness.py status`.

Submit every time the best jumps, not just at the end. **Only what is on HackerRank counts.**

## 7. Timeline

| When | Goal |
|------|------|
| 0-10 min | Read all problems, split them up, paste statements, launch agents |
| by ~40 min | A valid submission on EVERY problem (even a weak one) |
| next | 70%+ on every main problem. This unlocks the bonus |
| then | Improve the weakest problem first. Bonus only once all four are 70%+ |
| last 10 min | No new experiments. Confirm HackerRank holds every `best/pN/BEST.cpp` |

## 8. Practice run (do this before 18:00, ~45 min)

`p0` is a practice problem with a scorer and tests already built.
```
cd ~/c3 && ./agents.sh p0-a:claude p0-b:codex
```
Type `You are agent p0-a.` in one window and `You are agent p0-b.` in the other.
Goal: get from 0.046 to above 0.5. Practise switching windows, answering prompts, running
`python3 harness.py status`, and copying `best/p0/BEST.*` with `clip.exe`.
Afterwards, reset the practice scores with `rm -rf best`.

## 9. What the harness output means

- `NEW BEST 0.62 ... -> best/p1/BEST.cpp` - improvement saved. Submit it.
- `KEPT OLD BEST` - this attempt wasn't better; nothing changed. Normal.
- `INVALID: ...` - the answer broke a rule on that test (scores 0). The agent must fix it.
- `TIMEOUT` / `SLOW` - too close to the time limit. Ask the agent to lower `time_budget_s`.
- `No checker at ...` - agent `a` hasn't built the checker yet.
