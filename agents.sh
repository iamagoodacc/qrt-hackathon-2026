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
  tmux set-option -t "$SESSION" mouse on   # click window names to switch; wheel scrolls; Shift+drag copies
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
