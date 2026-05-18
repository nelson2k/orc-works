# Legacy And Integrations

`src/helpers/pymupdf_rag.py` is the older extraction implementation. It converts PDF pages to GitHub-flavored Markdown using PyMuPDF text extraction, multi-column helpers, table detection, image/vector extraction, and font-size-based header detection.

Use the legacy path with:

```python
import pymupdf4llm

pymupdf4llm.use_layout(False)
```

Legacy-only heading helpers:

- `IdentifyHeaders(doc, pages=None, body_limit=12, max_levels=6)` maps font sizes to Markdown heading prefixes.
- `TocHeaders(doc)` uses the PDF table of contents to identify headings.

LlamaIndex integration:

```python
reader = pymupdf4llm.LlamaMarkdownReader()
docs = reader.load_data("document.pdf")
```

The reader in `src/llama/pdf_markdown_reader.py` imports `llama_index`, opens the PDF with PyMuPDF, and returns one LlamaIndex `Document` per page with metadata such as page number, total pages, file path, and document metadata.

Compatibility package:

- `pdf4llm/` is a small package that imports and re-exports `pymupdf4llm`.
- Its README describes the older `pdf4llm.to_markdown(...)` package name.

Examples:

- `examples/country-capitals` demonstrates table extraction feeding a simple RAG/chatbot flow.
- `examples/GUI` contains a browser GUI demo for a RAG chatbot.

