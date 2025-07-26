#!/usr/bin/env bash
set -euo pipefail

REPO_URL="https://github.com/pratiksingh94/mesh-c2.git"
DEST_DIR="${1:-mesh-c2}" # provide destination dir if you want but no need tbh mesh-c2 sounds cool asf
C2_ENV_FILE="C2/.env"


for cmd in git python3 docker; do
  if ! command -v "$cmd" &>/dev/null; then
    echo "❌ '$cmd' is required. Please install it and re-run."
    exit 1
  fi
done

if [[ -d "$DEST_DIR/.git" ]]; then
  echo "🔄 Repo already exists in '$DEST_DIR', pulling latest..."
  git -C "$DEST_DIR" pull --ff-only
else
  echo "📥 Cloning into '$DEST_DIR'..."
  git clone "$REPO_URL" "$DEST_DIR"
fi

for script in start.sh attach-logs.sh install.sh; do
  if [[ -f "$DEST_DIR/$script" ]]; then
    chmod +x "$DEST_DIR/$script"
    echo "⚙️  chmod +x $script"
  fi
done


if [[ ! -f "$DEST_DIR/$C2_ENV_FILE" ]]; then
  echo "🚧 Creating default C2/.env"
  cat > "$DEST_DIR/$C2_ENV_FILE" <<EOF
# C2 server listening port
PORT=8000
EOF
else
  echo "🔍 Found existing $C2_ENV_FILE, skipping"
fi

cat <<EOF

✅ Installation complete!

➡️ Next steps:

   cd $DEST_DIR

   # Tweak C2 port if you want:
   # edit $C2_ENV_FILE and change PORT=...

   # Start the whole mesh:
   ./start.sh [NUM_IMPLANTS] [-v]

   # In another terminal, tail all logs:
   ./attach-logs.sh

EOF
