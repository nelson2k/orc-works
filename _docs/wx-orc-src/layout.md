# Layout

```
wx-ocr-src/
├── CMakeLists.txt
├── requirements.txt       # Python deps for the worker
├── worker.py              # Python OCR worker (entry point for the child process)
├── venv/                  # Python virtualenv (gitignored)
├── build/                 # CMake build output (gitignored)
├── icons/                 # SVG icons copied next to the exe at build time
│   ├── OCR_toolbar_icon.svg     # app/window icon, blue
│   ├── chevron-left.svg / chevron-right.svg
│   ├── file-text.svg / files.svg
│   └── folder-open.svg
└── src/
    ├── main.cpp           # MainFrame, App, layout, event wiring
    ├── Worker.h/.cpp      # Spawns, talks to, and cancels the Python worker
    ├── Metrics.h/.cpp     # CPU / RAM / GPU sampling
    ├── VBar.h/.cpp        # Vertical metric bar custom control
    ├── ZoomPanel.h/.cpp   # Scrolled, Ctrl+wheel zoomable preview
    ├── FlatButton.h/.cpp  # Flat toolbar button with optional icon
    ├── icon.ico           # Multi-resolution ICO for the Win32 resource
    └── orcgui.rc          # Win32 resource: icon + wx manifest
```

Runtime path resolution:

- the exe lives at `wx-ocr-src/build/orcgui.exe`
- [Worker.cpp](../../wx-ocr-src/src/Worker.cpp) resolves the Python and
  worker paths **relative to the exe**, not cwd, via `GetModuleFileNameW`,
  so the app can be launched from anywhere:
  - Python  →  `<exe-dir>\..\venv\Scripts\python.exe`
  - Worker  →  `<exe-dir>\..\worker.py`
- output Markdown is written to `<repo-root>/output/<pdf-stem>/` by
  `worker.py` (which computes `REPO_ROOT` as `dirname(__file__)/..`)
- icons live in an `icons/` folder next to the exe; a post-build CMake
  step copies `wx-ocr-src/icons` there. SVGs are loaded at runtime by
  `wxBitmapBundle::FromSVGFile`.

The only thing still outside the folder is the wxWidgets build at
`repos-folder/wxWidgets-install/`, consumed at compile time — see
[build.md](build.md).
