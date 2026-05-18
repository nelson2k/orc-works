# API

Public functions are exported from `src/__init__.py`.

Core extraction functions:

```python
import pymupdf4llm

md = pymupdf4llm.to_markdown("document.pdf")
data = pymupdf4llm.to_json("document.pdf")
text = pymupdf4llm.to_text("document.pdf")
```

Inputs can be file paths or `pymupdf.Document` objects. `pages` accepts an integer or a sequence of zero-based page numbers.

Common options:

- `pages=[...]` - extract selected pages.
- `page_chunks=True` - return one dictionary per page instead of one combined string.
- `write_images=True` - write extracted image files and reference them.
- `embed_images=True` - embed extracted images as data URIs.
- `image_path`, `image_format`, `dpi` / `image_dpi` - control image output.
- `header=False`, `footer=False` - omit detected page headers/footers in layout mode.
- `ignore_code=True` - avoid code-style formatting.
- `show_progress=True` - show progress on larger page sets.

OCR options:

- `use_ocr=False` disables OCR.
- `force_ocr=True` OCRs every PDF page when an OCR callback is available.
- `ocr_dpi=300` controls OCR rasterization.
- `ocr_language="eng"` is passed to OCR engines that use language data.
- `ocr_function=my_func` supplies a custom page OCR callback.

Other public helpers:

- `use_layout(True|False)` toggles the modern layout path.
- `get_key_values(doc, xrefs=False)` extracts PDF form fields.
- `markdown_to_pdf(md_path, ..., output_path=None)` renders Markdown to a PyMuPDF PDF document or file.
- `LlamaMarkdownReader()` returns the LlamaIndex reader wrapper.

When layout mode is disabled with `use_layout(False)`, legacy helpers `IdentifyHeaders` and `TocHeaders` become available for custom heading detection.

