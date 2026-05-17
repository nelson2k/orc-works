#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python_bin="$repo_root/marker-code/venv/bin/python"
requirements="$repo_root/backend/requirements.txt"

if [[ ! -x "$python_bin" ]]; then
  echo "Python venv not found: $python_bin" >&2
  exit 1
fi

"$python_bin" -m pip install -r "$requirements"
