# Layout Pipeline

Default mode is layout mode. On import, `src/__init__.py` tries to import `pymupdf.layout` and calls `pymupdf.layout.activate()`. If that import fails, it falls back to the legacy `pymupdf_rag.py` path.

Main flow:

```text
to_markdown / to_json / to_text
  -> _layout_to_*
    -> helpers.document_layout.parse_document
      -> pymupdf.open(...)
      -> optional image-to-PDF conversion
      -> optional PDF StructTreeRoot removal
      -> per page:
        -> remove page rotation
        -> analyze OCR need
        -> optional OCR callback
        -> get textpage / blocks
        -> page.get_layout()
        -> clean pictures / tables
        -> find reading order
        -> table extraction or table fallback image
        -> build PageLayout and LayoutBox objects
    -> ParsedDocument.to_markdown / to_json / to_text
```

Key dataclasses in `document_layout.py`:

- `LayoutBox` - one layout item with bbox, class, optional image bytes/path, table data, and text lines.
- `PageLayout` - page number, size, boxes, OCR flags, text blocks, words, and links.
- `ParsedDocument` - filename, page count, TOC, metadata, form fields, image settings, OCR mode, and page list.

Layout box classes are handled semantically: `title`, `section-header`, `list-item`, `footnote`, normal text, `picture`, `formula`, `table`, `table-fallback`, `page-header`, and `page-footer`.

Serialization behavior:

- Markdown preserves headings, lists, inline styles, code blocks, tables, and image references.
- JSON serializes the full parsed document, including layout boxes and metadata.
- Plain text converts tables through `tabulate` and can wrap table cells.

`page_chunks=True` returns per-page dictionaries with `metadata`, `toc_items`, `page_boxes`, and `text`. `page_boxes` includes the box class, bbox, and character-position range inside the page text.

