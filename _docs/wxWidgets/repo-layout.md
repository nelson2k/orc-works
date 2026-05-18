# Repository layout

Top-level of `repos-folder/wxWidgets`:

| Path | Purpose |
|------|---------|
| `include/wx/` | Public C++ headers (437 entries at the root + per-port subdirs) |
| `src/` | Library implementation, organized by port (`msw`, `gtk`, `osx`, `qt`, `x11`, `univ`, `dfb`, `unix`, `common`, ...) |
| `interface/` | Doxygen-only header stubs used to generate API docs |
| `samples/` | ~95 example programs (`minimal`, `widgets`, `aui`, `xrc`, `webview`, ...) |
| `demos/` | Larger demonstration apps |
| `tests/` | Test suite |
| `utils/` | Helper executables (e.g. `wxrc`, image converters) |
| `docs/` | Per-port build docs and Doxygen sources — see [docs-tree.md](docs-tree.md) |
| `build/` | Build system files: `cmake/`, `msw/` (vcxproj, sln, nmake), `bakefiles/`, helper scripts |
| `art/` | Built-in art (Tango icons, etc.) |
| `locale/` | Translation `.po` files |
| `3rdparty/` | Bundled third-party code |
| `lib/` | Output / placeholders for built libraries |
| `misc/` | Misc bits (debugger visualizers, fonts, etc.) |
| `CMakeLists.txt`, `CMakePresets.json` | CMake entry points |
| `configure`, `configure.ac`, `Makefile.in` | Autoconf entry points |
| `wxwidgets.props` | MSBuild props for downstream apps |
| `setup.h.in` | Build-time feature toggles template |

The two top-level READMEs are `README.md` and `README-GIT.md` (extra notes for
people building from a Git checkout instead of a tarball).
