# Usage

## From a downstream CMake project

This install is **monolithic** — only `wx::mono` is exposed as a
component target. The conventional `wx::core` / `wx::base` aliases are
not created:

```cmake
find_package(OpenGL REQUIRED)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS mono)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE wx::mono)
```

`wx::mono` carries everything (`wxUSE_GUI=1`, `__WXMSW__`, every
subsystem's headers and link deps).

The explicit `find_package(OpenGL REQUIRED)` is mandatory: this install
was built with `USE_OPENGL=1`, so `wx::mono`'s link interface references
`OpenGL::GL`, but `wxWidgetsConfig.cmake` only auto-imports OpenGL when
the separate `wx::wxgl` component target exists (and in monolithic
builds it does not). See [cmake-config.md](cmake-config.md).

CONFIG mode is what this install supports. The legacy non-CONFIG
`FindwxWidgets` (which scans for `wx-config`) is not what you want here —
this install has no `wx-config` script.

## Pointing CMake at this install

Set one of these so `find_package(wxWidgets CONFIG)` discovers the
config file:

```bash
# Option A: pass on the command line
cmake -S . -B build -DwxWidgets_DIR="<repo-root>/repos-folder/wxWidgets-install/lib/cmake/wxWidgets-3.3"

# Option B: env var
export wxWidgets_DIR="<repo-root>/repos-folder/wxWidgets-install/lib/cmake/wxWidgets-3.3"

# Option C: CMAKE_PREFIX_PATH (also picks up other find_packages)
cmake -S . -B build -DCMAKE_PREFIX_PATH="<repo-root>/repos-folder/wxWidgets-install"
```

`wxWidgets_DIR` must point at the directory **containing**
`wxWidgetsConfig.cmake`, not at the install prefix root.

## Toolchain requirements

This install is built for a single toolchain triple:

- Compiler family: **gcc**
- Architecture: **x86_64**
- Linkage: **static**

A consumer must match. On MSYS2: use the `mingw64` shell (or have
`C:\msys64\mingw64\bin` on PATH so `c++.exe` is the mingw64 gcc).
MSVC or clang-cl consumers will not find a matching target subdir.

## Static linking — no DLLs to ship

Because the build is static, the produced executable has no
`wxmsw*.dll` / image / compression DLL runtime dependencies from wx.
It still depends on the platform CRT and the MSYS2 runtime
(`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`)
because `RUNTIME_LIBS=dynamic` in `build.cfg`.

## Components you can request

For this install the only valid component names are:

```
mono        # the monolithic library (wx::wxmono)
base_only   # GUI-less variant (wx::wxbase_only, defines wxUSE_GUI=0)
```

Requesting `core`, `base`, `aui`, etc. fails with `wxWidgets_FOUND =
FALSE` because no matching `wx::wx<name>` target was installed.
