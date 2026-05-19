# Layout

```
wx-ocr-src/
├── CMakeLists.txt
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

Runtime assumptions (same as `go-ocr-src/`):

- the exe is launched from `wx-ocr-src/build/.../` (or similar)
- it resolves Python as `..\venv\Scripts\python.exe`
- it resolves the worker as `..\worker.py`
- output Markdown is written to `<repo-root>/output/<pdf-stem>/`
- icons live in an `icons/` folder next to the exe; a post-build CMake step
  copies `wx-ocr-src/icons` there

Both paths above are hardcoded in [worker.md](worker.md).
