# Build

## Prerequisites

- **MinGW-w64 g++** with C++17 support (the project uses `set(CMAKE_CXX_STANDARD 17)`)
- **CMake** ≥ 3.16
- **Ninja**
- **wxWidgets 3.3**, built from source and installed at
  `../repos-folder/wxWidgets-install/`. That path is hard-coded as the
  default in [CMakeLists.txt:10-12](../../wx-ocr-src/CMakeLists.txt#L10-L12).
- **OpenGL** dev libs (mingw bundle)
- **Python 3.x venv** at `wx-ocr-src/venv/` for the Local-mode worker.
  See [worker-stdio.md](worker-stdio.md) and the
  `requirements-1.txt` / `requirements-2.txt` two-stage install
  ([gotchas.md](gotchas.md) explains why).

## What CMake fetches

`FetchContent` pulls **nlohmann/json v3.11.3** at configure time
([CMakeLists.txt:15-21](../../wx-ocr-src/CMakeLists.txt#L15-L21)).
Everything else is found locally.

## Link line

`wx::mono` (monolithic wxWidgets), `nlohmann_json::nlohmann_json`,
and on Windows: `dwmapi` (dark caption / border), `winhttp` (Remote
mode HTTP+SSE).
([CMakeLists.txt:42-47](../../wx-ocr-src/CMakeLists.txt#L42-L47))

The exe is built with the `WIN32` flag, so it's a windowed app (no
console).
([CMakeLists.txt:40](../../wx-ocr-src/CMakeLists.txt#L40))

Compile definitions on Windows: `_UNICODE UNICODE NOMINMAX WIN32_LEAN_AND_MEAN`.
([CMakeLists.txt:50-53](../../wx-ocr-src/CMakeLists.txt#L50-L53))

## Resource compile

`src/orcgui.rc` is added as a source on Windows. The RC compile is given
two include dirs so `#include "wx/msw/wx.rc"` resolves: the wxWidgets
public headers and the `mswu/` setup-header dir.
([CMakeLists.txt:32-38](../../wx-ocr-src/CMakeLists.txt#L32-L38))

## Post-build step

The `icons/` directory is copied next to the exe so SVG icons can be
loaded at runtime relative to `wxStandardPaths::Get().GetExecutablePath()`.
([CMakeLists.txt:56-60](../../wx-ocr-src/CMakeLists.txt#L56-L60))
At runtime [main.cpp:42-48](../../wx-ocr-src/src/main.cpp#L42-L48) reads
from `<exe-dir>/icons/<name>.svg`.

## Configure + build

From the project root (`017-ocr-works/`):

```powershell
cmake -S wx-ocr-src -B wx-ocr-src\build -G Ninja
cmake --build wx-ocr-src\build
```

Or use [`run.ps1`](../../run.ps1) — it configures, builds, and launches
the exe in one shot.

## Output

`wx-ocr-src/build/orcgui.exe`, with `wx-ocr-src/build/icons/*.svg` next
to it.

## Worker startup path

In Local mode Worker.cpp resolves the project root as `<exe-dir>\..\`
and launches `<root>\venv\Scripts\python.exe <root>\worker.py`.
([Worker.cpp:22-31](../../wx-ocr-src/src/Worker.cpp#L22-L31),
[Worker.cpp:287-289](../../wx-ocr-src/src/Worker.cpp#L287-L289))
So the layout assumes the exe lives at `wx-ocr-src/build/orcgui.exe`.
