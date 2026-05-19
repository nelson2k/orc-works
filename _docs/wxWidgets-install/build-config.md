# Build configuration

The flavor is recorded in
`lib/gcc_x64_lib/mswu/build.cfg`:

```
WXVER_MAJOR=3
WXVER_MINOR=3
WXVER_RELEASE=3
BUILD=release
MONOLITHIC=1
SHARED=0
UNICODE=1
TOOLKIT=MSW
WXUNIV=0
VENDOR=custom
DEBUG_FLAG=Default
DEBUG_INFO=0
RUNTIME_LIBS=dynamic
USE_EXCEPTIONS=1
USE_RTTI=1
USE_THREADS=1
USE_AUI=1
USE_GUI=1
USE_HTML=1
USE_MEDIA=
USE_OPENGL=1
USE_QA=1
USE_PROPGRID=1
USE_RIBBON=1
USE_RICHTEXT=1
USE_STC=1
USE_WEBVIEW=1
USE_XRC=1
COMPILER=gcc
CC=cc
CXX=c++
```

What this implies for consumers:

- **Single archive to link** (`MONOLITHIC=1`) — every feature lives in
  `wxmsw33u.a`. No need to enumerate component libs by hand; just link
  `wx::core` (or `wx::wxmono` directly) and the chain pulls everything.
- **Static link to wx** (`SHARED=0`) — no `wxmsw*.dll` redistributable
  needed. The consuming exe carries wx code inside.
- **Dynamic CRT** (`RUNTIME_LIBS=dynamic`) — links against the mingw
  runtime DLLs (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`,
  `libwinpthread-1.dll`); those must be findable at run time, or
  static-link the runtime yourself in your downstream project.
- **Unicode only** (`UNICODE=1`, `mswu` tag) — `wxString` is wide, all
  APIs are the Unicode variants. No ANSI build is available.
- **No debug build** — `mswud/` exists but is empty. CMake's Debug
  config will still try to use Release artifacts via the standard
  `MAP_IMPORTED_CONFIG_*` fallback, but symbols won't match. For real
  debugging, rebuild wx with `BUILD=debug`.
- **MSW toolkit, not WXUniv** — native Win32 controls; no portable
  emulation widgets.
- **No media** (`USE_MEDIA=` is empty) — `wxMediaCtrl` is unavailable.
- **OpenGL on** — `wx::gl` is usable; consumers get `find_dependency(OpenGL)`.
- **No curl** — `wxWebRequest` over HTTPS is not wired (the
  `find_dependency(CURL)` clause in `wxWidgetsConfig.cmake` is
  short-circuited by `AND OFF`).

## setup.h note

`lib/gcc_x64_lib/mswu/wx/setup.h` is the per-build feature header. CMake
adds its parent directory to include paths so `<wx/setup.h>` resolves to
this file (rather than the unconfigured stub in `include/wx/`).

The macro `wxINSTALL_PREFIX` in this file is baked to the original
build-time path:

```c
#define wxINSTALL_PREFIX "C:/Users/nelso/dev/001-markplayer/repos-folder/wxWidgets-install"
```

This is used at run time only — for locating translation catalogs etc.
when no explicit path is given to wx. Build and link work regardless
because CMake paths come from `_IMPORT_PREFIX` (the file's actual
location), not from `wxINSTALL_PREFIX`.
