# Headers (`include/wx/`)

`include/wx/` has **437 entries**. Most are flat `wx/<feature>.h` headers;
per-port specifics live in subdirectories.

## The umbrella header

`wx/wx.h` is the "include almost everything common" header. Used by samples
when precompiled headers aren't enabled. It pulls in: `defs`, `object`,
`string`, `event`, `app`, `window`, `frame`, `dialog`, `sizer`, common
controls, drawing (`dc`, `bitmap`, `image`, `pen`, `brush`), menus, etc.

For PCH-friendly code use `wx/wxprec.h` first, then either rely on `wx/wx.h`
in the non-PCH branch or include only the headers you need.

## Per-port subdirectories

| Subdir | Port |
|--------|------|
| `wx/msw/` | Windows native |
| `wx/gtk/` | GTK+ 2 & 3 |
| `wx/osx/` | macOS (Cocoa) |
| `wx/qt/` | Qt-based port |
| `wx/x11/` | Pure X11 port |
| `wx/dfb/` | DirectFB |
| `wx/univ/` | wxUniversal (renders its own widgets) |
| `wx/android/` | Android (experimental) |
| `wx/unix/` | Shared Unix bits |
| `wx/generic/` | Generic (non-native) widget implementations |
| `wx/private/` | Internal-only headers |
| `wx/meta/` | Template metaprogramming helpers |
| `wx/persist/`, `wx/propgrid/`, `wx/ribbon/`, `wx/richtext/`, `wx/stc/`, `wx/aui/`, `wx/html/`, `wx/xml/`, `wx/xrc/`, `wx/protocol/` | Subsystem-specific headers |

## Notable single-feature headers

- Core: `app.h`, `event.h`, `window.h`, `frame.h`, `dialog.h`, `panel.h`,
  `sizer.h`, `gbsizer.h`, `wrapsizer.h`
- Controls: `button.h`, `checkbox.h`, `combobox.h`, `listctrl.h`,
  `treectrl.h`, `grid.h`, `dataview.h`, `notebook.h`, `treelist.h`
- Drawing: `dc.h`, `dcclient.h`, `dcmemory.h`, `dcsvg.h`, `graphics.h`,
  `image.h`, `bitmap.h`, `bmpbndl.h` (DPI-aware bitmap bundle)
- I/O: `file.h`, `ffile.h`, `filename.h`, `stream.h`, `stdpaths.h`
- Networking: `socket.h`, `url.h`, `webrequest.h`, `webview.h`
- System: `thread.h`, `timer.h`, `intl.h` (i18n), `uilocale.h`, `log.h`
- Misc: `xrc/` (XML UI), `aui/` (advanced UI / docking), `stc/` (Scintilla
  text control), `propgrid/` (property grid), `richtext/`, `ribbon/`
