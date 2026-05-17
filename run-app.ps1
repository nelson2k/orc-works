#requires -Version 7
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$Python = Join-Path $RepoRoot 'marker-code\venv\Scripts\python.exe'
$Frontend = Join-Path $RepoRoot 'frontend'
$BackendLog = Join-Path $RepoRoot 'backend\backend.log'

if (-not (Test-Path $Python)) {
    throw "Python venv not found: $Python"
}

if (-not (Test-Path $Frontend)) {
    throw "Frontend folder not found: $Frontend"
}

Write-Host "Starting backend at http://127.0.0.1:8000"
Write-Host "Backend log: $BackendLog"

$backend = Start-Process `
    -FilePath $Python `
    -ArgumentList @('-m', 'uvicorn', 'backend.app.main:app', '--host', '127.0.0.1', '--port', '8000', '--reload') `
    -WorkingDirectory $RepoRoot `
    -RedirectStandardOutput $BackendLog `
    -RedirectStandardError $BackendLog `
    -PassThru `
    -WindowStyle Hidden

try {
    Write-Host "Starting frontend at http://localhost:5173"
    Write-Host "Press Ctrl+C to stop the frontend. The backend will be stopped automatically."
    npm --prefix frontend run dev
}
finally {
    if ($backend -and -not $backend.HasExited) {
        Write-Host "Stopping backend..."
        Stop-Process -Id $backend.Id -Force
    }
}
