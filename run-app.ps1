#requires -Version 7
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$Python = Join-Path $RepoRoot 'marker-code\venv\Scripts\python.exe'
$Frontend = Join-Path $RepoRoot 'frontend'

if (-not (Test-Path $Python)) {
    throw "Python venv not found: $Python"
}

if (-not (Test-Path $Frontend)) {
    throw "Frontend folder not found: $Frontend"
}

$backendArgs = @(
    '-NoExit',
    '-Command',
    "Set-Location '$RepoRoot'; & '$Python' -m uvicorn backend.app.main:app --host 127.0.0.1 --port 8000 --reload"
)

$frontendArgs = @(
    '-NoExit',
    '-Command',
    "Set-Location '$RepoRoot'; npm --prefix frontend run dev"
)

Start-Process pwsh -ArgumentList $backendArgs -WindowStyle Normal
Start-Process pwsh -ArgumentList $frontendArgs -WindowStyle Normal

Write-Host "Backend starting at http://127.0.0.1:8000"
Write-Host "Frontend starting at http://localhost:5173"
