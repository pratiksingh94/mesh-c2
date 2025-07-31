#!/usr/bin/env bash
set -euo pipefail

if ! command -v stdbuf &>/dev/null; then
  echo "stdbuf not found—installing coreutils..."
  if   command -v pacman   &>/dev/null; then
    sudo pacman -S --needed --noconfirm coreutils
  elif command -v apt-get  &>/dev/null; then
    sudo apt-get update && sudo apt-get install -y coreutils
  else
    echo "⚠️  Could not auto-install stdbuf. Please install coreutils manually" >&2
    exit 1
  fi
fi


SESSION="mesh-logs"

if ! command -v tmux &>/dev/null; then
  cat <<EOF
❌ tmux not found. View logs manually:

1) Install tmux:
   - Arch/Manjaro: sudo pacman -S tmux  
   - Debian/Ubuntu: sudo apt-get install tmux

2) Or view logs manually in separate terminals:
   # C2 server logs
   tail -f logs/c2.log

   # Implant logs
   docker logs -f implant1
   docker logs -f implant2
   ...
EOF
  exit 0
fi


NUM=$(docker ps --filter "name=implant" --format "{{.Names}}" | grep -E "^implant[0-9]+" | wc -l)
if [[ "$NUM" -eq 0 ]]; then
  echo "⚠️ No implant containers detected. Did you forget to run ./start.sh?"
  exit 1
fi

tmux kill-session -t "$SESSION" &>/dev/null || true


tmux new-session -d -s "$SESSION" \
  "tail -f logs/c2.log | stdbuf -oL sed 's/^/[C2] /'"


for i in $(seq 1 "$NUM"); do
  tmux new-window -t "$SESSION" -n "implant$i" \
    "docker logs -f implant$i | stdbuf -oL sed 's/^/[$i] /'"
done

tmux select-window -t "$SESSION:0"
tmux attach -t "$SESSION"
