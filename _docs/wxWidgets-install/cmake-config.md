# CMake Config

The package is consumed in **CONFIG mode** — the modern path — not via
the older `FindwxWidgets` module that scans for `wx-config`.

## Files

- `lib/cmake/wxWidgets-3.3/wxWidgetsConfig.cmake` — entry point.
- `lib/cmake/wxWidgets-3.3/wxWidgetsConfigVersion.cmake` — declares
  `PACKAGE_VERSION "3.3.3"` and the standard major-minor compatibility
  rule.
- `lib/cmake/wxWidgets-3.3/gcc_x64_lib/wxWidgetsTargets.cmake` —
  defines the imported targets (`add_library(... STATIC IMPORTED)`).
- `lib/cmake/wxWidgets-3.3/gcc_x64_lib/wxWidgetsTargets-release.cmake` —
  fills in `IMPORTED_LOCATION_RELEASE` paths for each `.a`.

## How it picks the right subdir

`wxWidgetsConfig.cmake` chooses `${wxCOMPILER_PREFIX}${wxARCH_SUFFIX}_lib`
based on the consuming project:

- `wxCOMPILER_PREFIX` — `vc` for MSVC, `gcc` for GNU, `clang` for Clang.
- `wxARCH_SUFFIX` — `_x64` on 64-bit, empty on 32-bit.
- `_dll` vs `_lib` — shared vs static; this install ships only `_lib`.

For consumers on MSYS2 mingw64 (gcc, 64-bit), it resolves to
`gcc_x64_lib/wxWidgetsTargets.cmake`. Cross-toolchain consumption
(e.g. MSVC) is not supported by this install — only one
`(compiler, arch, linkage)` triple is present.

## Imported targets

Created directly by `wxWidgetsTargets.cmake`:

```
wx::wxmono       wx::wxbase_only
wx::wxregex      wx::wxzlib       wx::wxexpat
wx::wxjpeg       wx::wxpng        wx::wxtiff
wx::webp         wx::webpdemux    wx::sharpyuv
wx::wxscintilla  wx::wxlexilla
```

For each `wx::wx<name>` target that **is present** in the targets file,
`wxWidgetsConfig.cmake` adds the short aliases `wx::<name>` and
`wxWidgets::<name>`. Aliases are **not** created for components whose
underlying target is missing — so for this monolithic-only install,
the only short aliases that exist are:

```
wx::mono       wxWidgets::mono       (-> wx::wxmono)
wx::base_only  wxWidgets::base_only  (-> wx::wxbase_only)
```

The conventional per-component names (`wx::core`, `wx::base`, `wx::aui`,
`wx::html`, …) are **absent**. Downstream code must link `wx::mono`,
which transitively pulls in everything the monolithic archive exposes.

It also builds a catch-all interface target `wxWidgets::wxWidgets` that
links every found component, and fills the legacy
`wxWidgets_LIBRARIES` variable for `FindwxWidgets` compatibility.

## Required dependencies pulled in transitively

`wxWidgetsConfig.cmake` calls:

- `find_dependency(Threads)` — always.
- `find_dependency(OpenGL)` — **gated on `TARGET wx::wxgl`**, which is
  not created in monolithic-only installs (see below).
- `find_dependency(CURL)` — gated behind a `AND OFF` clause, so
  effectively disabled in this install (no `wx::wxnet` HTTPS via curl).

## Monolithic + OpenGL gotcha

Because `USE_OPENGL=1` in `build.cfg`, the `wx::wxmono` target's
`INTERFACE_LINK_LIBRARIES` includes `OpenGL::GL` and `opengl32 / glu32`.
But the OpenGL `find_dependency` in `wxWidgetsConfig.cmake` is guarded
by `if(TARGET wx::wxgl ...)`, and `wx::wxgl` is **not** present in a
monolithic build (only `wx::wxmono` is). So the include of
`wxWidgetsTargets.cmake` fails with:

```
CMake Error ... wxWidgetsTargets.cmake:146 (set_target_properties):
  The link interface of target "wx::wxmono" contains:
    OpenGL::GL
  but the target was not found.
```

Workaround for downstream consumers: call `find_package(OpenGL REQUIRED)`
**before** `find_package(wxWidgets CONFIG REQUIRED COMPONENTS mono)`.
