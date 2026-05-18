# Worker Engines

`ocr(path, page, engine)` dispatches extraction.

`digital`:

- uses `pymupdf4llm.to_markdown`
- intended for born-digital PDF pages
- uses no GPU model memory

`marker`:

- loads Marker models with `create_model_dict`
- builds a Marker document by hand
- emits stages for layout, line detection, OCR, structure, processors,
  render, and saving
- emits a page layout overlay PNG after layout detection

`marker_llm`:

- uses the Marker path with `use_llm=True`
- configures `marker.services.openai.OpenAIService`
- reads `MARKER_LLM_BASE_URL`, `MARKER_LLM_MODEL`, and
  `MARKER_LLM_API_KEY`
- defaults to `http://localhost:8080/v1`, model `local`, and key `sk-no-key`

`vlm`:

- loads Qwen2.5-VL from `OCR_VLM_PATH` or the default repo-local model path
- rasterizes the page at 200 DPI
- prompts the model to produce GitHub-flavored Markdown only
- decodes the generated text and saves it as page Markdown

`auto`:

- first checks for enough digital text with PyMuPDF
- uses `digital` when the page appears born-digital
- otherwise runs Marker
- asks Surya's OCR error predictor whether Marker text is bad
- falls back to VLM when quality is bad and the VLM model directory exists
