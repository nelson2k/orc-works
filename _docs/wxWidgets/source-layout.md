# Source layout (`src/`)

The implementation is split into one directory per port plus shared code:

| Directory | What's in it |
|-----------|--------------|
| `src/common/` | Cross-platform implementation shared by every port (string, event, sizer, controls' base classes, streams, etc.). Hundreds of `.cpp` files; many end in `cmn.cpp`. |
| `src/generic/` | "Generic" widget implementations written in wxWidgets itself; used when a port doesn't have a native equivalent. |
| `src/msw/` | Windows native widgets, drawing, DDE, OLE, dark mode, custom-draw, etc. |
| `src/gtk/` | GTK+ (2 / 3) widgets. |
| `src/osx/` | macOS Cocoa widgets. |
| `src/qt/` | Qt-backed widgets. |
| `src/x11/`, `src/dfb/` | X11 direct and DirectFB ports. |
| `src/univ/` | wxUniversal — wxWidgets renders its own controls. Useful for embedded / theming. |
| `src/unix/` | Unix-only helpers used by GTK/Qt/X11 ports. |
| `src/html/` | Lightweight HTML renderer (`wxHtmlWindow`). |
| `src/xml/`, `src/xrc/` | XML support + the XRC resource format. |
| `src/aui/`, `src/ribbon/`, `src/propgrid/`, `src/richtext/`, `src/stc/` | Subsystem implementations. |
| `src/png/`, `src/jpeg/`, `src/tiff/`, `src/expat/`, `src/zlib/` | Bundled third-party codecs / parsers. |

## Pattern

For most controls there's a triple:

- `include/wx/<thing>.h` — public API and forward declarations
- `src/common/<thing>cmn.cpp` — shared logic (validation, sizing, defaults)
- `src/<port>/<thing>.cpp` — native binding for a port

Example: `wxButton` has `wx/button.h`, `src/common/btncmn.cpp`, and
`src/msw/button.cpp` / `src/gtk/button.cpp` / `src/osx/.../button.mm`.
