# Build

`CMakeLists.txt` targets CMake 3.16+, C++17, no extensions.

Dependencies:

- **wxWidgets 3.3**, resolved via the bundled install at
  `../repos-folder/wxWidgets-install/lib/cmake/wxWidgets-3.3`.
  Linked as the `mono` component (single all-in-one wx library).
- **OpenGL** — `find_package(OpenGL REQUIRED)` (pulled in by wx setup).
- **nlohmann/json v3.11.3** — fetched via `FetchContent` from GitHub.
- Win32 system libs: `dwmapi` (dark caption/border), `psapi` (in `Metrics.cpp`).

The single executable target is `orcgui`, built as a `WIN32` (no console)
binary. On Windows the build:

- enables the `RC` language and compiles [src/orcgui.rc](../../wx-ocr-src/src/orcgui.rc)
  with `-I` flags pointing at the bundled wxWidgets headers and the
  `gcc_x64_lib/mswu` setup header dir
- defines `_UNICODE`, `UNICODE`, `NOMINMAX`, `WIN32_LEAN_AND_MEAN`
- runs a post-build step that copies `wx-ocr-src/icons/` next to the exe
