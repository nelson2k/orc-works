# Layout

```
go-ocr-src/
├── requirements.txt
├── worker.py
├── venv/
└── gui/
    ├── go.mod
    ├── go.sum
    ├── main.go
    ├── metrics.go
    ├── vbar.go
    ├── zoom.go
    └── orcgui.exe
```

Runtime assumptions:

- the Go app is launched from `go-ocr-src/gui/`
- it resolves Python as `../venv/Scripts/python.exe`
- it resolves the worker as `../worker.py`
- output Markdown is written to `<repo-root>/output/<pdf-stem>/`
- the default VLM model path is
  `<repo-root>/repos-folder/Qwen2.5-VL-3B-Instruct-AWQ`

The `venv/` folder is local runtime state. The `orcgui.exe` file is a built
GUI binary.
