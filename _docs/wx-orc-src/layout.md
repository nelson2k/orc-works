# Layout

```
wx-ocr-src/
├── CMakeLists.txt
├── requirements.txt       # Python deps for the worker (copied from go-ocr-src)
├── worker.py              # Python OCR worker (copied from go-ocr-src)
├── venv/                  # Python virtualenv (gitignored)
├── build/                 # CMake build output (gitignored)
├── icons/                 # SVG icons copied next to the exe at build time
├── image.png
└── src/
    ├── main.cpp           # MainFrame, App, layout, event wiring
    ├── Worker.h/.cpp      # Spawns and talks to the Python worker
    ├── Metrics.h/.cpp     # CPU / RAM / GPU sampling
    ├── VBar.h/.cpp        # Vertical metric bar custom control
    ├── ZoomPanel.h/.cpp   # Scrolled, Ctrl+wheel zoomable image preview
    ├── FlatButton.h/.cpp  # Flat blue toolbar button with icon
    ├── icon.ico
    └── orcgui.rc          # Win32 resource: icon + wx manifest
```

Runtime assumptions:

- the exe lives at `wx-ocr-src/build/orcgui.exe`
- `Worker.cpp` resolves Python and worker paths **relative to the exe**,
  not the current working directory, via `GetModuleFileNameW` — so it can
  be launched from anywhere:
  - Python  →  `<exe-dir>\..\venv\Scripts\python.exe`
  - Worker  →  `<exe-dir>\..\worker.py`
- output Markdown is written to `<repo-root>/output/<pdf-stem>/` (computed
  by `worker.py` as `os.path.dirname(__file__) + "/../output"`)
- icons live in an `icons/` folder next to the exe; a post-build CMake step
  copies `wx-ocr-src/icons` there

The folder is self-contained at runtime — the C++ code, the Python worker,
its venv, and the SVG icons are all inside `wx-ocr-src/`. The only thing
still outside is the wxWidgets build, which lives in `repos-folder/` and
is consumed at compile time (see [build.md](build.md)).
