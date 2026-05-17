# Chandra — Docs Index

Chandra OCR 2 (by Datalab) is a single vision-language model that converts
images and PDFs into structured HTML / Markdown / JSON with layout info. Code
is Apache-2.0; model weights use a modified OpenRAIL-M license.

Source repo: `repos-folder/chandra/`.

## Pipeline at a glance

```
file (pdf / image)
  └─ load_file → list[PIL.Image]   (input.py)
      └─ BatchInputItem(image, prompt_type="ocr_layout")
          └─ InferenceManager.generate
              ├─ hf  → transformers AutoModelForImageTextToText  (model/hf.py)
              └─ vllm → OpenAI-compatible chat call to vLLM       (model/vllm.py)
          └─ raw HTML string (one big <div>-per-block layout)
              └─ output.parse_{html,markdown,layout,chunks}
              └─ output.extract_images (crops from page image using bboxes)
```

## Pages

- [overview.md](overview.md) — what chandra is, single-model architecture
- [install.md](install.md) — pip extras, from source
- [cli.md](cli.md) — `chandra` CLI flags and output layout
- [backends.md](backends.md) — `hf` vs `vllm` trade-offs
- [pipeline.md](pipeline.md) — per-page request flow
- [schema.md](schema.md) — `BatchInputItem`, `BatchOutputItem`, `GenerationResult`, `LayoutBlock`
- [prompts.md](prompts.md) — `OCR_LAYOUT_PROMPT`, allowed labels and HTML tags
- [output.md](output.md) — HTML → markdown conversion + image extraction
- [settings.md](settings.md) — env / `local.env` config
- [apps.md](apps.md) — `chandra_app` (streamlit), `chandra_vllm` (docker), `chandra_screenshot` (flask)
- [vs_marker.md](vs_marker.md) — how chandra differs from marker
