# Building with CMake

Top-level `CMakeLists.txt` requires CMake **3.10** or newer (capped at the
4.1 policy range). It parses the version from `include/wx/version.h` and
the SO version from `build/bakefiles/version.bkl`.

## Quick start

```
cmake -S . -B build
cmake --build build --config Release -j
```

On Windows the default generator is the latest installed MSVC. Pass
`-G "Ninja"` for a single-config build.

`CMakePresets.json` is provided at the top level — `cmake --list-presets`
to see them.

## macOS deployment targets

If `CMAKE_OSX_DEPLOYMENT_TARGET` is unset, the build picks:

- iOS → 12.0
- macOS → 10.10

Set it before the first `project()` call to override.

## Useful options

(See `build/cmake/` and `docs/doxygen/overviews/cmake.md` for the full
list.)

- `wxBUILD_SHARED=ON|OFF` — shared (default) vs static
- `wxBUILD_MONOLITHIC=ON|OFF` — single library vs split per subsystem
- `wxBUILD_SAMPLES=ALL|SOME|OFF`
- `wxBUILD_TESTS=ALL|CONSOLE_ONLY|OFF`
- `wxBUILD_PRECOMP=ON|OFF` — PCH
- `wxUSE_*` — many feature switches matching `setup.h`

## Consuming wxWidgets from a downstream CMake project

```cmake
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
add_executable(myapp myapp.cpp)
target_link_libraries(myapp PRIVATE wx::core wx::base)
```

(or the legacy non-CONFIG mode that reads `wx-config` / registry).

## Auto-generated config headers

The build emits a `setup.h` from `setup.h.in`. Do not edit the generated
one — change `wxUSE_*` cache vars (CMake) or edit `include/wx/msw/setup.h`
(legacy MSVC build).
