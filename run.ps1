#requires -Version 5.1
# Build and run the tauri-orc-src desktop app in dev mode.
# Equivalent to: cd tauri-orc-src; npm install (if needed); npm run tauri dev

$ErrorActionPreference = 'Stop'

$appDir = Join-Path $PSScriptRoot 'tauri-orc-src'

if (-not (Test-Path $appDir)) {
    throw "tauri-orc-src/ not found at $appDir"
}

Push-Location $appDir
try {
    if (-not (Test-Path 'node_modules')) {
        Write-Host '> node_modules missing — running npm install' -ForegroundColor Cyan
        npm install
        if ($LASTEXITCODE -ne 0) { throw "npm install failed (exit $LASTEXITCODE)" }
    }

    if (-not (Test-Path 'marker-code\venv\Scripts\python.exe')) {
        Write-Warning 'marker-code/venv missing — the marker sidecar will fail to start. See _docs/tauri-orc-src/build-run.md.'
    }

    Write-Host '> npm run tauri dev' -ForegroundColor Cyan
    npm run tauri dev
} finally {
    Pop-Location
}
