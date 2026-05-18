$ErrorActionPreference = "Stop"

Push-Location "$PSScriptRoot\src\gui"
try {
    go run .
}
finally {
    Pop-Location
}
