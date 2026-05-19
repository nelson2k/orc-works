$ErrorActionPreference = "Stop"

$src = "$PSScriptRoot\wx-ocr-src"
$build = "$src\build"
$exe = "$build\orcgui.exe"

cmake -S $src -B $build -G Ninja
if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

cmake --build $build
if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }

Push-Location "$PSScriptRoot\go-ocr-src\gui"
try {
    & $exe
}
finally {
    Pop-Location
}
