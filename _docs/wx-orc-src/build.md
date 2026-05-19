# Build

`CMakeLists.txt` targets CMake 3.16+, C++17, no extensions. Run via
[wx-run.ps1](../../wx-run.ps1) from the repo root, which does:

1. `cmake -S wx-ocr-src -B wx-ocr-src/build -G Ninja`
2. `cmake --build wx-ocr-src/build`
3. launches `build/orcgui.exe`

## Dependencies

- **wxWidgets 3.3**, resolved via the bundled install at
  `../repos-folder/wxWidgets-install/lib/cmake/wxWidgets-3.3`. Linked as
  the `mono` component (single all-in-one wx library). Statically
  consumed — the runtime exe needs no wx DLLs.
- **OpenGL** — `find_package(OpenGL REQUIRED)` (pulled in by wx setup).
- **nlohmann/json v3.11.3** — fetched via `FetchContent` from GitHub
  into `build/_deps/`.
- Win32 system libs: `dwmapi` (dark caption/border) and `psapi`
  (memory metrics — referenced from `Metrics.cpp`).

## Windows specifics

- enables the `RC` language and compiles
  [src/orcgui.rc](../../wx-ocr-src/src/orcgui.rc) with `-I` flags
  pointing at the bundled wxWidgets headers and the
  `gcc_x64_lib/mswu` setup header dir
- compile defines: `_UNICODE`, `UNICODE`, `NOMINMAX`, `WIN32_LEAN_AND_MEAN`
- target is built as `WIN32` (subsystem:windows — no console)
- post-build step copies `wx-ocr-src/icons/` next to the exe
