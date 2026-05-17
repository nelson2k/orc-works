# chandra — notes

Source: `repos-folder/chandra` (datalab-to). Package name `chandra-ocr`, version `0.2.0`. License: Apache-2.0 code, OpenRAIL-M weights.

What it is: a vision-language OCR model that takes a page image and returns structured HTML (with bounding-box annotations on each layout block). Post-processing turns that HTML into markdown, plain HTML, or a chunked JSON layout list. The model is `datalab-to/chandra-ocr-2`, a fine-tuned Qwen3-VL-style image-text-to-text checkpoint.

How it works at a glance:

1. Input file (PDF or image) is rasterized to a list of PIL images (`chandra/input.py`).
2. Each image is wrapped as a `BatchInputItem(image, prompt_type="ocr_layout")`.
3. `InferenceManager.generate(batch)` runs the model — either locally via HuggingFace transformers (`generate_hf`) or remotely through a vLLM OpenAI-compatible server (`generate_vllm`).
4. The raw HTML returned by the model has `<div data-bbox="x0 y0 x1 y1" data-label="Section-Header">…</div>` blocks. Bboxes are normalized to 0–1000.
5. Post-processors split this into: `markdown` ([chandra/output.py: parse_markdown](../../repos-folder/chandra/chandra/output.py)), full `html`, and `chunks` (a flat list of `{bbox, label, content}` records).
6. Any `Image`/`Figure` blocks are cropped from the source image and saved under a hashed filename.

Two inference paths:

- **HuggingFace local** — `chandra ... --method hf`. Loads the model into VRAM via `transformers.AutoModelForImageTextToText`. Defaults to batch size 1.
- **vLLM server** — `chandra ... --method vllm`. Talks to a vLLM OpenAI-compatible endpoint (default `http://localhost:8000/v1`). Default batch size 28. `chandra_vllm` launches the server in a Docker container.
