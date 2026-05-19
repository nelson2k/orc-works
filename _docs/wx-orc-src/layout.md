# Folder layout

```
wx-ocr-src/
├── CMakeLists.txt        # build setup (see build.md)
├── worker.py             # Python worker, ~1090 lines, two entry points
├── requirements.txt      # legacy single-shot install
├── requirements-1.txt    # stage 1: torch + marker + transformers + autoawq
├── requirements-2.txt    # stage 2: mineru + fastapi + uvicorn (bumps Pillow)
├── venv/                 # Python venv (Local mode launches python.exe from here)
├── icons/
│   ├── OCR_toolbar_icon.svg
│   ├── chevron-left.svg / chevron-right.svg
│   ├── file-text.svg / files.svg
│   └── folder-open.svg
├── src/
│   ├── main.cpp          # wxApp/wxFrame + toolbar + Extract handlers
│   ├── Worker.h          # the transport abstraction
│   ├── Worker.cpp        # Local subprocess+pipes / Remote WinHTTP+SSE
│   ├── Metrics.h
│   ├── Metrics.cpp       # GetSystemTimes + GlobalMemoryStatusEx + nvidia-smi
│   ├── VBar.h / VBar.cpp # vertical metric bars (CPU/RAM/GPU/VRAM/TEMP)
│   ├── ZoomPanel.h
│   ├── ZoomPanel.cpp     # image zoom/pan, Ctrl-wheel zoom, fit-width
│   ├── FlatButton.h
│   ├── FlatButton.cpp    # flat colored button widget (drawn by hand)
│   ├── orcgui.rc         # Win32 icon resource + wx manifest
│   └── icon.ico          # the .ico embedded in the exe
└── build/                # CMake/Ninja output; orcgui.exe lands here
```

## Source pairing at a glance

| Source | What's in it |
| --- | --- |
| [main.cpp](../../wx-ocr-src/src/main.cpp) | `MainFrame`, toolbar, Open/Prev/Next/Extract Page/Extract PDF/Stop, metrics tick, scp, fit-to-width, page navigation, Ctrl/Space tracking |
| [Worker.cpp](../../wx-ocr-src/src/Worker.cpp) | `request()` dispatches on `Mode`, Local subprocess+pipes, Remote WinHTTP POST/SSE, cancel via `TerminateProcess` (Local) or close-request-handle (Remote), `pollRemoteMetricsHttp` thread |
| [Metrics.cpp](../../wx-ocr-src/src/Metrics.cpp) | CPU% from FILETIME deltas, RAM from `GlobalMemoryStatusEx`, GPU/VRAM/temp by shelling out to `nvidia-smi --query-gpu=...` |
| [VBar.cpp](../../wx-ocr-src/src/VBar.cpp) | One column with a label on top and a name on the bottom, fill rectangle between |
| [ZoomPanel.cpp](../../wx-ocr-src/src/ZoomPanel.cpp) | `wxScrolled<wxPanel>` with bilinear-scaled bitmap, Ctrl+wheel zoom, space-pan |
| [FlatButton.cpp](../../wx-ocr-src/src/FlatButton.cpp) | `wxPanel` painted as a flat button, three colors (normal/hover/pressed), emits `wxEVT_BUTTON` |

## Python entry points

- `main_stdio()` — Local mode. Reads JSON commands from stdin, writes
  JSON results to a dup of FD 1.
  ([worker.py:945-972](../../wx-ocr-src/worker.py#L945-L972))
- `main_http()` — Remote mode. Runs uvicorn on
  `OCR_HTTP_HOST:OCR_HTTP_PORT` (default `0.0.0.0:9000`).
  ([worker.py:1074-1078](../../wx-ocr-src/worker.py#L1074-L1078))

`main()` picks one based on the presence of `--http` in argv.
([worker.py:1081-1089](../../wx-ocr-src/worker.py#L1081-L1089))
