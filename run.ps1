#requires -Version 7
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$Venv     = Join-Path $RepoRoot 'marker-code\venv\Scripts\Activate.ps1'
$Script   = Join-Path $RepoRoot 'run.py'

if (-not (Test-Path $Venv))   { throw "Venv not found: $Venv" }
if (-not (Test-Path $Script)) { throw "Script not found: $Script" }

. $Venv

# Unbuffer python so prints + tqdm stream immediately to PowerShell
$env:PYTHONUNBUFFERED = '1'

python $Script @args
