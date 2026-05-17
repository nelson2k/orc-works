# Package layout

Contents of `repos-folder/chandra/chandra/`:

```
chandra/
├── __init__.py            (empty)
├── settings.py            pydantic-settings (env / local.env)
├── input.py               PDF/image → PIL.Image list
├── output.py              raw HTML → markdown / html / chunks / images
├── prompts.py             OCR prompts + allowed tags/attrs/labels
├── util.py                draw_layout debug helper
├── model/
│   ├── __init__.py        InferenceManager (top-level entry point)
│   ├── schema.py          BatchInputItem, BatchOutputItem, GenerationResult
│   ├── hf.py              HuggingFace backend (load_model, generate_hf)
│   ├── vllm.py            vLLM backend (generate_vllm) with retry/concurrency
│   └── util.py            scale_to_fit, detect_repeat_token
└── scripts/
    ├── __init__.py
    ├── cli.py             chandra command — file/folder OCR
    ├── vllm.py            chandra_vllm command — Docker vLLM launcher
    ├── run_app.py         chandra_app command — Streamlit wrapper
    ├── app.py             Streamlit app body
    ├── screenshot_app.py  chandra_screenshot command
    └── templates/         HTML templates for the apps
```

## Call graph

```
chandra (CLI)
  └─ chandra.input.load_file
  └─ chandra.model.InferenceManager.generate
       ├─ chandra.model.hf.generate_hf          (method="hf")
       │   └─ AutoModelForImageTextToText.generate
       └─ chandra.model.vllm.generate_vllm      (method="vllm")
           └─ openai.OpenAI.chat.completions.create  → vLLM server
       └─ chandra.output.parse_markdown / parse_html / parse_chunks / extract_images
```

## External dependencies actually used at runtime

- `pypdfium2` — PDF rasterization
- `pillow` — image handling
- `filetype` — file-type sniffing
- `beautifulsoup4` — HTML parsing in post-processing
- `markdownify` — HTML → markdown
- `openai` — vLLM client
- `pydantic` / `pydantic-settings` / `python-dotenv` — settings
- `click` — CLI
- `six` — used by the custom Markdownify subclass for text handling
- `transformers` / `torch` / `accelerate` — only for `method="hf"`
- `streamlit` — only for `chandra_app`

## Other top-level files in `repos-folder/chandra/`

- `README.md` — upstream docs
- `FULL_BENCHMARKS.md` — 90-language benchmark table
- `LICENSE` — Apache-2.0 (code)
- `MODEL_LICENSE` — OpenRAIL-M (weights)
- `pyproject.toml` — package config and entry points
- `uv.lock` — uv lockfile
- `pytest.ini`, `tests/` — test suite
- `assets/` — example images and benchmark plots
