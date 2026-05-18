# worker.py

`worker.py` is a long-lived Python process controlled by the Go GUI.

Core responsibilities:

- render PDF pages to PNG with PyMuPDF
- extract Markdown with `pymupdf4llm` for born-digital pages
- run Marker OCR
- optionally run Marker with an OpenAI-compatible LLM service
- run Qwen2.5-VL extraction for visual-language OCR
- save page Markdown and extracted images
- stream progress events back to the GUI

The worker patches `tqdm` before Marker and Surya import it. Progress bars are
converted into JSON `progress` events instead of terminal output.

Model state is resident:

- `_marker_models` caches Marker model artifacts
- `_vlm` caches the Qwen2.5-VL model and processor

The worker tries to keep only one heavy model family loaded at a time. Loading
Marker unloads the VLM; loading the VLM unloads Marker. CUDA cache cleanup is
attempted after unloading.

Saved Markdown path:

```text
output/<sanitized-pdf-stem>/page_NNNN.md
```
