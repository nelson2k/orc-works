# Dependencies

## Python — `src/requirements.txt`

```
--extra-index-url https://download.pytorch.org/whl/cu126

torch
torchvision
torchaudio

pymupdf
marker-pdf
```

- CUDA 12.6 wheels are pinned via the extra index URL — the project
  expects an NVIDIA card. CPU-only torch is also installable but
  marker is slow without GPU.
- `pymupdf` is used directly for page rasterization in
  `worker.render`.
- `marker-pdf` brings in `surya-ocr` and all downstream model
  packages. See [`_docs/marker/`](../marker/) and
  [`_docs/surya/`](../surya/).
- `tqdm` is not listed because marker pulls it in transitively;
  `worker.py` patches it at import time.

## Go — `src/gui/go.mod`

Direct requires:

```
fyne.io/fyne/v2                          v2.7.4
github.com/shirou/gopsutil/v4            v4.26.4
github.com/sqweek/dialog                 v0.0.0-20260123140253-...
```

Why each:

- **fyne/v2** — GUI toolkit. Reference in [`_docs/fyne/`](../fyne/).
- **gopsutil/v4** — `cpu.Percent` / `mem.VirtualMemory`, used by
  `metrics.go`.
- **sqweek/dialog** — native OS file picker (used for "Open PDF"
  instead of Fyne's in-app dialog).

Indirect deps are the standard Fyne fan-out (`go-gl`, `glfw`,
`go-text`, etc.) plus gopsutil's `wmi` / `go-ole` chain on Windows.

## Module name

`module orcgui` — note the typo (`orc` not `ocr`), matching the built
binary `orcgui.exe`. Not worth renaming since nothing imports the
module externally.
