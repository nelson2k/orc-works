$ErrorActionPreference = "Stop"

Push-Location "$PSScriptRoot\go-ocr-src\gui"
try {
    go run .
}
finally {
    Pop-Location
}
