# Resources

## orcgui.rc

[orcgui.rc](../../wx-ocr-src/src/orcgui.rc) is short — three things:

```
orcgui ICON "icon.ico"

#define wxUSE_RC_MANIFEST 1
#define wxUSE_DPI_AWARE_MANIFEST 2

#include "wx/msw/wx.rc"
```

- `orcgui ICON "icon.ico"` — embeds `src/icon.ico` as the application
  icon. `main.cpp` loads it via `SetIcon(wxICON(orcgui))`
  ([main.cpp:170-172](../../wx-ocr-src/src/main.cpp#L170-L172)).
- `wxUSE_DPI_AWARE_MANIFEST 2` — sets the per-monitor v2 DPI awareness
  on the embedded manifest.
- `#include "wx/msw/wx.rc"` — pulls in wxWidgets' standard MSW
  resources (manifest, common-controls, …).

The CMake config sets the RC include path so this resolves:
[CMakeLists.txt:35-37](../../wx-ocr-src/CMakeLists.txt#L35-L37).

## Embedded icon

`src/icon.ico` is the multi-resolution icon embedded in the executable.

## Runtime SVG icons

Separate from the .ico file, the toolbar buttons load SVG icons at
runtime from `<exe-dir>/icons/` (copied there by the CMake post-build
step — [build.md](build.md)).

| File | Used by | File |
| --- | --- | --- |
| `OCR_toolbar_icon.svg` | window icon bundle (16/24/32/48/64/128/256) | [main.cpp:174-193](../../wx-ocr-src/src/main.cpp#L174-L193) |
| `folder-open.svg` | Open PDF button | [main.cpp:206](../../wx-ocr-src/src/main.cpp#L206) |
| `file-text.svg` | Extract Page button | [main.cpp:219](../../wx-ocr-src/src/main.cpp#L219) |
| `files.svg` | Extract PDF button | [main.cpp:220](../../wx-ocr-src/src/main.cpp#L220) |
| `chevron-left.svg` | Prev button | [main.cpp:224](../../wx-ocr-src/src/main.cpp#L224) |
| `chevron-right.svg` | Next button | [main.cpp:226](../../wx-ocr-src/src/main.cpp#L226) |

[`loadIcon`](../../wx-ocr-src/src/main.cpp#L42-L48) wraps
`wxBitmapBundle::FromSVGFile` at a `20×20` default size; the button
draws it scaled to match the row height
([FlatButton.cpp:115-120](../../wx-ocr-src/src/FlatButton.cpp#L115-L120)).

## Window icon bundle

At construction the frame builds a `wxIconBundle` from the toolbar SVG
at sizes 16/24/32/48/64/128/256 and calls `SetIcons(bundle)` so the
taskbar / Alt-Tab / window list all get sharp icons regardless of DPI
([main.cpp:174-193](../../wx-ocr-src/src/main.cpp#L174-L193)).
