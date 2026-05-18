# Dependencies

Python dependencies in `requirements.txt`:

- PyTorch stack from the CUDA 12.6 wheel index
- `pymupdf`
- `pymupdf4llm`
- `marker-pdf`
- `transformers>=4.49`
- `accelerate`
- `autoawq`

The Qwen2.5-VL engine needs a local model directory. By default the worker
looks for:

```text
repos-folder/Qwen2.5-VL-3B-Instruct-AWQ
```

Set `OCR_VLM_PATH` to override that path.

Go dependencies in `gui/go.mod`:

- `fyne.io/fyne/v2`: desktop GUI
- `github.com/shirou/gopsutil/v4`: CPU and RAM metrics
- `github.com/sqweek/dialog`: native PDF file picker

The Go module name is `orcgui`.
