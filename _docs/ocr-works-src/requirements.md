# src/requirements.txt

Python deps for the worker venv (`src/venv/`).

```
--extra-index-url https://download.pytorch.org/whl/cu126

torch
torchvision
torchaudio

pymupdf
marker-pdf
```

- `pymupdf` — used by [worker.py](../../src/worker.py) for the `render` command (rasterize a page to PNG) and page-count lookup.
- `marker-pdf` — primary OCR backend. Pulls in surya (layout, recognition, table-rec, detection, OCR-error predictors). The worker lazy-loads its models on first OCR request.
- `torch` / `torchvision` / `torchaudio` — pinned to the CUDA 12.6 wheel index so surya can run on the GPU.

Tesseract is not in the pipeline; no `tessdata` directory is needed and no Tesseract binary has to be on PATH.
