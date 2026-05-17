#requires -Version 7
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$Python = Join-Path $RepoRoot 'marker-code\venv\Scripts\python.exe'

if (-not (Test-Path $Python)) {
    throw "Python venv not found: $Python"
}

& $Python -m uvicorn backend.app.main:app --host 127.0.0.1 --port 8000 --reload
