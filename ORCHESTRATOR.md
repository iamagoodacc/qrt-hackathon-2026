# Orchestrator guide (for humans)

Your job is to be the manager, not the coder: start agents, tell each one its name, keep
them unblocked, check the scoreboard, and paste the best answers into HackerRank.

## 1. Who does what

Each problem gets 2 or 3 agents. Their name tells them their job (the rules are in
`AGENTS.md`, which they read automatically):

| Window | Tool   | Role             | What it does |
|--------|--------|------------------|--------------|
| `p1-a` | codex  | Builder          | Reads the problem, builds the local scorer (`checker.py`) and tests, writes a first valid answer, then STOPS (saves credits) |
| `p1-b` | claude | Main optimiser   | C++ "simulated annealing": keeps making small random changes and keeping the good ones |
| `p1-c` | claude | Second optimiser | Optional. Double-checks the scorer against the statement, then tries a completely different method |

Launch all three: `./agents.sh p1-a:codex p1-b:claude:high p1-c:claude`

**The brief is one line.** In each agent's window type, e.g.:
```
You are agent p1-a.
```
To steer an agent, add an idea: `You are agent p1-b. Idea: moves = swap two items.`

Claude does the main optimising. Codex only does the short builder job, because its credits are
limited. Once `p1-a` has stopped, you can reuse it for another problem: `You are agent p4-a.`

### Choosing the model and effort
Higher effort = the agent thinks longer per step: smarter, but slower and uses up credits faster.
Add it as a third part when launching: `name:tool:effort`.

| You type | What starts |
|----------|-------------|
| `p1-a:codex` | Codex, default effort |
| `p1-b:claude:high` | Claude, high effort (recommended for the main optimiser) |
| `p1-c:claude` | Claude, default effort |
| `p3-b:fable` | Claude on the Fable model |
| `p1-c:claude2` | Claude on your SECOND account (see section 10) |

Efforts: `low`, `medium`, `high`, `xhigh` (Claude/Fable also have `max`).

**Fable uses up your allowance very fast.** Run at most one Fable agent at a time. Save it for the
hardest problem, or as a rescue when a problem is stuck below 70%. Don't combine it with `max`.

Changing a running agent: in Claude type `/effort` or `/model`; in Codex type `/model`.

## 2. tmux in 2 minutes

tmux = one terminal holding many windows, one per agent.

- **Use the Ubuntu app (Windows Terminal), not VS Code's terminal.** VS Code steals `Ctrl-b`
  (it toggles its sidebar). Keep VS Code open beside it for reading files.
- **Start agents:** `cd ~/c3 && ./agents.sh p1-a:codex p1-b:claude:high`
- **The bar at the bottom** lists your windows, e.g. `0:home 1:p1-a 2:p1-b*`. The `*` marks
  the one you're looking at. **Click a name to switch to it.**
- **Scroll:** mouse wheel (press `q` to stop scrolling). **Copy text:** hold Shift and drag.
- **Keys** (press `Ctrl-b`, let go, then the key):
  - `w` - list all windows, pick one with the arrow keys + Enter
  - `n` / `p` - next / previous window
  - `&` - close the window you're in (press `y` to confirm)
  - `d` - hide tmux; agents keep running. Come back with `tmux attach -t c3`
- **Closed the terminal by accident?** tmux and your agents are still running in the
  background. Run `tmux attach -t c3` to get them back. (Re-running `./agents.sh` with the same
  names also just reopens them; it never makes duplicates.)
- **Add agents later:** go to the `home` window and run `./agents.sh p2-a:codex` again.
- **Agent window shows a plain `$` prompt?** The agent quit. Close the window (`Ctrl-b &`) and
  launch it again with `./agents.sh`, then re-brief it.
- **Close everything:** `tmux kill-session -t c3`
- **tmux annoying you?** Just open one terminal tab per agent and run `cd ~/c3 && claude`
  (or `codex --sandbox workspace-write`) in each. It works the same.

## 3. First launch of each tool

- **Codex** asks whether to trust the folder: say **yes**, or it won't read `AGENTS.md`.
- **Claude** starts in "auto" permission mode, so routine work (editing files, compiling,
  running the harness) doesn't ask you. It still stops to ask about anything risky. If it
  does ask, "Yes, and don't ask again" stops repeats.
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

**Checking progress never needs a pause.** Agents keep working while you look:
- `watch -n 30 python3 harness.py status` in `home` = live scoreboard (`Ctrl-C` stops watching).
- `tail best/log.txt` = every saved attempt: time, problem, score, file, idea.

**Pausing without breaking anything:**
- **Pause ALL agents gently:** in `home`, run `touch PAUSE`. Each agent finishes its current step
  (usually within a few minutes, at the end of its next test run), writes a 3-line status, and
  waits. **Resume all:** `rm PAUSE`.
- **Talk to one Claude agent without stopping it:** just type your message and press Enter while
  it works. It reads it at its next step.
- **Stop one agent right now:** press `Esc`. This is safe too: `best/` is only ever written by
  the harness after a complete run, so your best answers can't get damaged. At worst the agent's
  current experiment is half-done. Type `continue` and it finishes it.

**Agent not improving for 15 minutes?** Give it a new idea (list below). If it seems
confused, start it fresh: type `/clear` (Claude) or `/new` (Codex), then the one-line brief again.

Ideas to hand out when stuck:
- "Start the annealing from the current best answer instead of from scratch."
- "Try a different kind of move" (swap two things / move one thing / reverse a section).
- "Make each step faster: change the answer in place and undo rejected moves."
- "Restart from several different random starting answers and keep the best."
- "Tune the CONFIG temperatures: try start_temp 10x bigger and 10x smaller."
- "Look at which tests score lowest and focus on those."

### When is an agent "done"?
- **Builder (`a`)** is done when it says the checker and tests are ready and reports a first
  score, and `python3 harness.py status` shows a line for that problem. Then it stops by itself.
- **Optimisers (`b`, `c`) are never done.** They keep trying ideas until you stop them. After every
  attempt they print one line like `NEW BEST score=0.71 (best 0.65) ... idea=...`.
  A problem is "done enough" when its best is submitted on HackerRank, and it is "safe" at 70%+.

### What to look out for
| You see | Do this |
|---------|---------|
| A question, menu, or "Do you want to...?" | Answer it. The agent is frozen until you do |
| `NEW BEST` | Submit `best/pN/BEST.cpp` to HackerRank (section 6) |
| HackerRank score far from the local score | The checker is wrong: section 5 |
| `INVALID`, `zero=` above 0, `TIMEOUT`, `SLOW` | Tell the agent: "Fix the invalid/slow tests first" |
| Same best for 15 min | Give a new idea (list below), or try `fable` |
| Agent writing Python solutions, editing the checker (if not `a`), or working outside its folder | "Stop. Follow AGENTS.md." |
| "usage limit" / "rate limit" message | Out of Claude usage: section 10 |

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
cd ~/c3 && ./agents.sh p0-a:codex p0-b:claude:high
```
Type `You are agent p0-a.` in one window and `You are agent p0-b.` in the other.
(p0's scorer already exists, so `p0-a` just writes a simple answer and stops.)
Goal: get from 0.046 to above 0.5. Practise switching windows, answering prompts, running
`python3 harness.py status`, and copying `best/p0/BEST.*` with `clip.exe`.
Afterwards, reset the practice scores with `rm -rf best`.

## 9. What the harness output means

- `NEW BEST 0.62 ... -> best/p1/BEST.cpp` - improvement saved. Submit it.
- `KEPT OLD BEST` - this attempt wasn't better; nothing changed. Normal.
- `INVALID: ...` - the answer broke a rule on that test (scores 0). The agent must fix it.
- `TIMEOUT` / `SLOW` - too close to the time limit. Ask the agent to lower `time_budget_s`.
- `No checker at ...` - agent `a` hasn't built the checker yet.

## 10. Second Claude account (when one runs out)

Each account keeps its own login, so both can run at the same time.

**Set up once, before 18:00** (in the Ubuntu app):
```
cd ~/c3 && CLAUDE_CONFIG_DIR=~/.claude-2 claude
```
Log in with the SECOND account, say yes to trusting the folder, then type `/exit`.

**Use it:** launch with `claude2` instead of `claude`, e.g. `./agents.sh p1-c:claude2`.
Tip: put the `b` agents on your main account and `c` agents on `claude2`, so neither runs out as fast.

**An agent hit its usage limit?** Its work is saved in files (its code, `NOTES.md`, `best/`), so:
1. Close its window: `Ctrl-b &`, then `y`.
2. Relaunch the same name on the other account: `./agents.sh p1-b:claude2:high`
3. Brief it: `You are agent p1-b.` It reads its NOTES.md log and carries on.
