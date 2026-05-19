# Resources

Non-code assets in `wx-ocr-src/`.

## Icons

`wx-ocr-src/icons/` — Lucide SVG icons loaded at runtime by
`wxBitmapBundle::FromSVGFile` at a fixed 20 × 20 logical size:

| File                 | Used for         |
|----------------------|------------------|
| `folder-open.svg`    | Open PDF button  |
| `file-text.svg`      | Extract Page     |
| `files.svg`          | Extract PDF      |
| `chevron-left.svg`   | Prev page        |
| `chevron-right.svg`  | Next page        |

Resolved relative to the running exe — see `loadIcon()` in
[main.cpp](main.md). CMake copies the folder next to the exe as a
post-build step (see [build.md](build.md)).

## Win32 resource

`src/orcgui.rc` declares one icon resource and pulls in `wx/msw/wx.rc`,
which provides the manifest with `wxUSE_RC_MANIFEST 1` and
`wxUSE_DPI_AWARE_MANIFEST 2`. That's what makes the app per-monitor DPI
aware on Windows.

The compiled icon is referenced from C++ via `wxICON(orcgui)` in
`MainFrame`'s constructor.

`src/icon.ico` is the source for the resource.
