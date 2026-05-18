# src/ layout

```
src/
├── requirements.txt        Python deps (torch + pymupdf + marker-pdf)
├── venv/                   local virtualenv (not documented)
├── worker.py               Python OCR worker (long-lived subprocess)
└── gui/
    ├── go.mod / go.sum     Fyne + gopsutil + sqweek/dialog
    ├── main.go             Fyne window, worker driver, button wiring
    ├── metrics.go          CPU/RAM sampling + nvidia-smi parser
    ├── vbar.go             vertical-bar widget used in the metrics column
    ├── zoom.go             ctrl+wheel zoom / space-drag pan wrapper
    └── orcgui.exe          built binary (gitignored output)
```

Runtime layout: the Go binary is launched from `src/gui/`, and resolves
`..\venv\Scripts\python.exe` and `..\worker.py` relative to its cwd. OCR
outputs are written under `<repo>/output/<pdf-stem>/`.

## Two-process design

- **Go GUI** owns the window, image viewer, navigation, metrics column.
- **Python worker** owns PDF rendering (PyMuPDF) and OCR (marker-pdf +
  surya models). It is a single long-lived process spoken to over
  stdin/stdout in newline-delimited JSON.

See [worker-protocol.md](worker-protocol.md) for the wire format.

## Per-file docs

- [worker.md](worker.md) — `worker.py`
- [worker-protocol.md](worker-protocol.md) — JSON commands and events
- [gui-main.md](gui-main.md) — `gui/main.go`
- [gui-metrics.md](gui-metrics.md) — `gui/metrics.go`
- [gui-vbar.md](gui-vbar.md) — `gui/vbar.go`
- [gui-zoom.md](gui-zoom.md) — `gui/zoom.go`
- [dependencies.md](dependencies.md) — `requirements.txt` + `go.mod`
