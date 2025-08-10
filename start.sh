#!/usr/bin/env bash
set -uo pipefail
# trap 'EXIT_CODE=$?; [[ $EXIT_CODE -ne 0 && $EXIT_CODE -ne 130 ]] && echo "❌ Error at line $LINENO: \`$BASH_COMMAND\` failed (exit code: $EXIT_CODE)" >&2' ERR


C2DIR="${C2DIR:-C2}"
IMPLANTDIR="${IMPLANTDIR:-implant}"
LOGDIR="${LOGDIR:-logs}"
NUM=3
VERBOSE=false

[[ -d "$C2DIR" ]]   || { echo "❌ C2 dir missing: $C2DIR" >&2; exit 1; }
[[ -d "$IMPLANTDIR" ]] || { echo "❌ implant dir missing: $IMPLANTDIR" >&2; exit 1; }
[[ -f "$IMPLANTDIR/include/config.h" ]] \
   || { echo "❌ config.h not found" >&2; exit 1; }


while [[ "$#" -gt 0 ]]; do
  case "$1" in
    -v|--verbose)
      VERBOSE=true
      ;;
    [0-9]*)
      NUM="$1"
      ;;
    *)
      echo "❌ Unknown argument: $1"
      exit 1
      ;;
  esac
  shift
done


t_log() {
  if [[ "$VERBOSE" == "true" ]]; then
    echo "[DEBUG] $*"
  fi
}


for cmd in python3 docker; do
  command -v $cmd &>/dev/null || { echo "❌ '$cmd' not found. Install and retry." >&2; exit 1; }
  t_log "Found $cmd"
done

mkdir -p "$LOGDIR"

cleanup() {
  echo
  echo "🛑 Shutting down mesh..."
  [[ -n "${C2_PID-}" ]] && kill "$C2_PID" 2>/dev/null && t_log "Killed C2 PID $C2_PID"
  for i in $(seq 1 "$NUM"); do
    docker rm -f "implant$i" &>/dev/null && t_log "Removed container implant$i"
  done

  tmux has-session -t mesh-logs 2>/dev/null && tmux kill-session -t mesh-logs
}
trap cleanup EXIT
trap 'exit' INT TERM
# trap '' SIGINT

echo "🕛 Starting C2..."
pushd "$C2DIR" >/dev/null
python3 -m venv venv
source venv/bin/activate
#pip install -q --upgrade -r requirements.txt
nohup python server.py > "../$LOGDIR/c2.log" 2>&1 &
C2_PID=$!
popd >/dev/null

sleep 2

echo "🎮 C2 launched (PID $C2_PID). Checking health..."

PORT=$(grep -Eo '^PORT=[0-9]+' "$C2DIR/.env" | cut -d= -f2 || echo "8000")
HOST=$(ip route get 8.8.8.8 2>/dev/null \
  | awk '{ for(i=1;i<NF;i++) if($i=="src") print $(i+1) }' \
  | head -n1)
HOST=${HOST:-127.0.0.1}
C2_URL="http://${HOST}:${PORT}"

until curl -sf "$C2_URL/" >/dev/null; do
  t_log "Waiting for C2 at $C2_URL..."
  sleep 1
done

echo "✅ C2 healthy at $C2_URL, logs -> $LOGDIR/c2.log"

echo ""
echo "🔗 Using C2_URL = $C2_URL"

sed -i -E "s|^#define[[:space:]]+C2_URL.*$|#define C2_URL \"${C2_URL}\"|" \
  "$IMPLANTDIR/include/config.h"
echo "💉 Injected C2_URL into $IMPLANTDIR/include/config.h"



echo ""
echo "🐳 Building implant image..."
DOCKER_BUILDKIT=1 docker build -q -t mesh-implant "$IMPLANTDIR" >/dev/null


echo "💀 Spawning $NUM implants..."
for i in $(seq 1 "$NUM"); do
  NAME="implant$i"
  docker rm -f "$NAME" &>/dev/null || true
  CID=$(docker run -d --name "$NAME" mesh-implant)
  echo "   ➡️ $NAME ($CID)"
  t_log "Launched $NAME as $CID"
done


echo
cat << EOF
✅ Mesh up and running!
- To STOP everything: Ctrl-C (kills C2 and all implants)
- To VIEW live logs: open another terminal here and run ./attach-logs.sh
EOF


# sleep infinity
while true; do
  sleep 1
done
