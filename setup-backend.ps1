#requires -Version 7
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$Python = Join-Path $RepoRoot 'marker-code\venv\Scripts\python.exe'
$Requirements = Join-Path $RepoRoot 'backend\requirements.txt'

if (-not (Test-Path $Python)) {
    throw "Python venv not found: $Python"
}

& $Python -m pip install -r $Requirements
