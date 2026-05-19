# wx-ocr-src

Windows-only C++/wxWidgets 3.3 desktop OCR app plus a Python worker that
does the actual page rendering and text extraction.

## What it does

Opens a PDF, lets you flip through pages with a zoomable preview, and runs
one of five OCR engines against the current page (or every page):

- **Auto** — digital-text shortcut, else Marker, else falls back to VLM if Marker output looks bad
- **Marker** — [`marker-pdf`](https://github.com/VikParuchuri/marker) layout + OCR pipeline
- **Marker + LLM** — Marker with an OpenAI-compatible LLM cleanup pass
- **VLM (Qwen2.5-VL)** — vision-language model transcribes the page image to Markdown
- **MinerU** — `mineru[pipeline]` (layout + OCR + formula + table)

See [engines.md](engines.md).

## Two backend modes

Selectable from a toolbar dropdown:

- **Local** — orcgui.exe spawns `venv\Scripts\python.exe worker.py` as a
  child process, talks to it over Win32 anonymous pipes (one JSON object
  per line on stdin/stdout).
- **Remote** — orcgui talks HTTP+SSE to a FastAPI worker at
  `http://192.168.10.200:9000` (a 4070 box running `worker.py --http`
  under systemd as `ocr-worker.service`). The PDF is scp'd to `/tmp/`
  once per Open.

See [backends.md](backends.md), [worker-stdio.md](worker-stdio.md),
[worker-http.md](worker-http.md).

## Build

Requires MinGW g++, CMake, Ninja, and a wxWidgets 3.3 install at
`../repos-folder/wxWidgets-install/`. From the project root:

```
.\run.ps1
```

…configures, builds, and launches `wx-ocr-src/build/orcgui.exe`.

Details: [build.md](build.md).

## Doc layout

| File | Topic |
| --- | --- |
| [build.md](build.md) | Build prerequisites, CMake invocation, where the binary lands |
| [layout.md](layout.md) | Folder/file tour |
| [backends.md](backends.md) | Local vs Remote, the toggle, scp, metrics flow |
| [worker-stdio.md](worker-stdio.md) | Local-mode JSON protocol |
| [worker-http.md](worker-http.md) | FastAPI endpoints, SSE format, systemd |
| [engines.md](engines.md) | The five engines and the Auto quality gate |
| [vlm.md](vlm.md) | llama-server vs transformers/AWQ runtime selection |
| [main.md](main.md) | main.cpp: frame, toolbar, Extract loops, fit-to-width |
| [worker-cpp.md](worker-cpp.md) | Worker.cpp: dispatch, WinHTTP/SSE, cancel, metrics thread |
| [metrics.md](metrics.md) | Local Metrics.cpp + remote SSE/poll pattern |
| [controls.md](controls.md) | Keyboard, mouse, dropdowns, stop button |
| [resources.md](resources.md) | orcgui.rc, the icon set |
| [gotchas.md](gotchas.md) | Worth remembering before touching the code |
