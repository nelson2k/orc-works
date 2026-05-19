# wxWidgets-install

Notes on the `repos-folder/wxWidgets-install/` folder.

This is a **pre-built, installed copy of wxWidgets 3.3.3** ready for CMake
`find_package(wxWidgets CONFIG REQUIRED)` consumption. It is the
output of `cmake --install` against a wx build tree, not the wx source
tree itself.

Key facts:

- Version: **3.3.3**
- Compiler: **gcc** (MinGW-style; MSYS2 mingw64 layout)
- Build type: **release**, **monolithic**, **static**, **Unicode**, MSW toolkit
- Runtime libs linkage: dynamic CRT
- Optional features enabled: AUI, HTML, OpenGL, PropGrid, Ribbon, RichText, STC, WebView, XRC, QA

The CMake config files target an `_IMPORT_PREFIX` derived from their own
location, so the install tree is **relocatable** — moving the folder
does not break `find_package`. The `wxINSTALL_PREFIX` macro baked into
`setup.h` still references the original build path, but that's a runtime
string only used for asset lookup, not for build or link.

## Documents

- [layout.md](layout.md): directory tree.
- [libraries.md](libraries.md): the `.a` archives and what each one is.
- [cmake-config.md](cmake-config.md): `wxWidgetsConfig.cmake`, targets, components.
- [usage.md](usage.md): how to consume this install from a downstream CMake project.
- [build-config.md](build-config.md): `build.cfg` and what the build flavor implies.
- [includes-and-locale.md](includes-and-locale.md): `include/wx/` and `share/locale/`.
