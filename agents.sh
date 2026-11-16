#!/usr/bin/env bash
# Launch coding agents, one tmux window each, all started from the repo root
# (so every agent reads AGENTS.md / CLAUDE.md and can run harness.py).
#
# Usage:   ./agents.sh p1-a:codex p1-b:claude:high p1-c:claude
#          name:tool[:effort]  tool = codex | claude | claude2 (2nd account) | fable (Fable model)
#          effort = low | medium | high | xhigh (claude/fable also max). Leave out for the default.
#          A name that is already open is skipped, so re-running never makes duplicates.
#
# tmux keys (press Ctrl-b, let go, then the key):
#   w   pick a window from a list           n / p   next / previous window
#   &   close this window (y to confirm)    d   hide tmux, agents keep running (back: tmux attach -t c3)
# Close everything: tmux kill-session -t c3
set -euo pipefail

SESSION="${SESSION:-c3}"
ROOT="$(cd "$(dirname "$0")" && pwd)"
CODEX_CMD="${CODEX_CMD:-codex --sandbox workspace-write}"
CLAUDE_CMD="${CLAUDE_CMD:-claude --permission-mode auto}"   # auto = fewer permission prompts

if [ "$#" -eq 0 ]; then
  sed -n '2,13p' "$0"
  exit 1
fi

if ! tmux has-session -t "$SESSION" 2>/dev/null; then
  tmux new-session -d -s "$SESSION" -n home -c "$ROOT"
  tmux set-option -t "$SESSION" mouse on   # click window names to switch; wheel scrolls; Shift+drag copies
fi

for spec in "$@"; do
  IFS=: read -r name tool effort <<< "$spec"
  case "$tool" in
    codex)  cmd="$CODEX_CMD${effort:+ -c model_reasoning_effort=$effort}" ;;
    claude) cmd="$CLAUDE_CMD${effort:+ --effort $effort}" ;;
    # second Claude account: its own login lives in ~/.claude-2 (log in once: see ORCHESTRATOR.md)
    claude2) cmd="CLAUDE_CONFIG_DIR=$HOME/.claude-2 $CLAUDE_CMD${effort:+ --effort $effort}" ;;
    fable)  cmd="$CLAUDE_CMD --model fable${effort:+ --effort $effort}" ;;
    *) echo "unknown tool '$tool' in '$spec' (use codex, claude, claude2 or fable)"; exit 1 ;;
  esac
  if tmux list-windows -t "$SESSION" -F '#{window_name}' | grep -qxF -- "$name"; then
    echo "'$name' is already open - skipped (to restart it: close it with Ctrl-b & first)"
    continue
  fi
  # "; exec bash" keeps the window open if the agent exits, so you can read what happened
  tmux new-window -t "$SESSION" -n "$name" -c "$ROOT" "$cmd; exec bash"
done

if [ -n "${TMUX:-}" ]; then
  echo "Added windows to session '$SESSION'. Switch with Ctrl-b w."
else
  tmux attach -t "$SESSION"
fi
