#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python_bin="$repo_root/marker-code/venv/bin/python"

if [[ ! -x "$python_bin" ]]; then
  echo "Python venv not found: $python_bin" >&2
  exit 1
fi

"$python_bin" -m uvicorn backend.app.main:app --host 127.0.0.1 --port 8000 --reload
