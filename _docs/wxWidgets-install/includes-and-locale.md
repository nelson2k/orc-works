# Includes and locale data

## include/wx/

The standard wxWidgets public header tree. ~424 top-level entries in
`include/wx/` itself, plus subdirectories per subsystem and per
platform.

What you `#include` directly:

```cpp
#include <wx/wx.h>          // catch-all "everything common" header
#include <wx/wxprec.h>      // precompiled-header friendly entry point
```

Or pick-and-choose for faster compiles:

```cpp
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/filedlg.h>
#include <wx/scrolwin.h>
#include <wx/splitter.h>
#include <wx/mstream.h>
#include <wx/timer.h>
```

Subdirectories of note (each is a feature area; many headers live in
each):

- `aui/` — advanced UI (dockable panels, notebook).
- `html/` — wxHTML rendering engine.
- `msw/` — Windows-specific headers (toolkit native types).
- `propgrid/` — property grid.
- `ribbon/` — ribbon UI controls.
- `richtext/` — rich text control + serialization.
- `stc/` — Scintilla-based source editor.
- `webview/` — embedded web view.
- `xrc/` — XML resource loader.
- `protocol/`, `xml/`, `private/`, etc.

The platform-specific `setup.h` for **this build** lives at
`lib/gcc_x64_lib/mswu/wx/setup.h` and is added to the include path by
the CMake target — `#include <wx/setup.h>` resolves there, not under
`include/wx/`.

## share/locale/

46 locale directories, each containing `LC_MESSAGES/` with the wx
translation catalogs (`.mo` files) for standard messages: stock button
labels, common-dialog text, file picker labels, etc.

Locales present:

```
af   an   ar   ca   ca@valencia   co   cs   da   de   el   es
eu   fa_IR  fi   fr   gl_ES   hi   hr   hu   id   ...
```

(46 total, including many EU/CJK variants.)

To use them at run time:

```cpp
wxLocale loc;
loc.Init(wxLANGUAGE_DEFAULT);
loc.AddCatalogLookupPathPrefix(".../wxWidgets-install/share/locale");
loc.AddCatalog("wxstd");
```

If you skip this, wx's stock dialogs render in English regardless of
the system locale. Apps that ship their own translations typically
copy this `share/locale/` tree alongside the executable.
