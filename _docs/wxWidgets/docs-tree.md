# Upstream `docs/` tree

For deeper detail, the upstream `docs/` directory is the source of truth.

## Per-port build & install docs

| Path | Topic |
|------|-------|
| `docs/msw/install.md` | Windows builds (MSVC / MinGW / nmake / vcpkg) — see [build-msw.md](build-msw.md) |
| `docs/msw/binaries.md` | Where to get pre-built MSW binaries |
| `docs/msw/msys2-msw.md`, `msys2-gtk.md`, `msys2-qt.md`, `gtk.md` | MSYS2 variants under Windows |
| `docs/gtk/`, `docs/osx/`, `docs/qt/`, `docs/x11/`, `docs/dfb/`, `docs/wine/`, `docs/ios/`, `docs/vms/`, `docs/univ/` | Per-port install notes |
| `docs/base/` | Non-GUI (`wxBase`) usage |

## Release / change notes

- `docs/changes.txt` — current branch
- `docs/changes_32.txt` — 3.2 series
- `docs/changes_30.txt` — 3.0 series
- `docs/release.md` — how upstream cuts a release

## License & legal

- `docs/licence.txt` — full text of the wxWindows licence
- `docs/lgpl.txt`, `docs/gpl.txt`, `docs/preamble.txt`, `docs/licendoc.txt`
- See [license.md](license.md) for the short version.

## API documentation

- `docs/doxygen/` — Doxyfile, layout, custom CSS, regen scripts
- `docs/doxygen/overviews/` — long-form topic guides (events, sizers,
  threading, dnd, xrc, persistence, high DPI, internationalization, ...)
- `docs/doxygen/groups/` — group definitions used to organise the API
  reference

## Internal technical notes

- `docs/tech/` — design notes for contributors

## Contribution

- `docs/contributing/` — how to submit patches and write commit messages
