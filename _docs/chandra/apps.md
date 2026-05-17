# Apps and helper scripts

Beyond the `chandra` CLI, the repo ships three smaller entry points.

## `chandra_app` — Streamlit demo

`chandra/scripts/run_app.py` → wraps `streamlit run chandra/scripts/app.py`.

Single-page UI:

- Sidebar: pick mode (`None` / `hf` / `vllm`), upload a PDF or image, pick a
  page if PDF.
- Main: left pane has three tabs — rendered HTML+images, raw HTML, layout
  overlay (red bboxes with blue labels). Right pane shows the uploaded image.
- "Run OCR" button triggers `model.generate([item])`.
- Markdown gets a `Download Markdown` button.
- Images are embedded as base64 data-URLs so everything works without
  filesystem access.

Models are `@st.cache_resource()`'d so the load only happens once.

## `chandra_vllm` — Docker vLLM launcher

`chandra/scripts/vllm.py`. Wraps `sudo docker run vllm/vllm-openai:v0.17.0`
with sensible flags for the chandra model.

```bash
chandra_vllm                    # H100 defaults
chandra_vllm --gpu 4090         # 24 GB consumer card
chandra_vllm --gpu 4090 --mtp   # +MTP speculative decoding (unstable)
```

Supported GPUs (preset VRAM): `h100, a100-80, a100/a100-40, l40s, a10, l4,
4090, 3090, t4`. `max-num-batched-tokens` and `max-num-seqs` are scaled
linearly from H100 baselines (8192 tokens, 64 seqs) by `vram / 80`.

Important flags hard-coded:
- `--no-enforce-eager`
- `--dtype bfloat16`
- `--max-model-len 18000`
- `--gpu-memory-utilization 0.85`
- `--enable-prefix-caching`
- `--mm-processor-kwargs '{"min_pixels": 3136, "max_pixels": 6291456}'`
- `--served-model-name <VLLM_MODEL_NAME>`

Mounts `~/.cache/huggingface` into the container so model weights aren't
re-downloaded.

## `chandra_screenshot` — Flask demo

`chandra/scripts/screenshot_app.py` — minimal Flask app on `0.0.0.0:8503`
designed for capturing "before/after" screenshots: original page with
colour-coded bbox overlays on the left, extracted markdown on the right.

Always uses `method="vllm"` (model loaded lazily on first request). Uses a
fixed colour palette per block label (Text=blue, Section-Header=teal,
Table=yellow, Figure=tan, etc.). Returns a JSON payload with base64 image,
block list, HTML, and markdown; the template (`scripts/templates/`) renders
the layout.

POST endpoint: `/process` with `{"file_path": "...", "page_number": 0}`.

Use case: producing the visual examples you see in the README / benchmarks.
