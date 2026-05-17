# Pipeline

Entry point is a converter's `__call__`. For `PdfConverter` (the default) the flow is in [marker/converters/pdf.py](../../repos-folder/marker/marker/converters/pdf.py):

1. `filepath_to_str` — if the caller passes a `BytesIO`, dumps it to a temp `.pdf`.
2. `build_document(path)`:
   - `provider_from_filepath(path)` picks a provider from sniffed file type (PDF / image / docx / xlsx / pptx / epub / html).
   - `resolve_dependencies` injects the right surya models from `artifact_dict` into each builder.
   - `DocumentBuilder(provider, layout_builder, line_builder, ocr_builder)` returns a `Document` of `PageGroup`s.
   - `StructureBuilder(document)` adds caption/figure groups and list groups.
   - Every processor in `default_processors` is run on the document in order.
3. `renderer(document)` produces the final pydantic output object (`MarkdownOutput`, `HTMLOutput`, `JSONOutput`, etc.).

`DocumentBuilder` itself does three things in order, see [marker/builders/document.py](../../repos-folder/marker/marker/builders/document.py):

```
layout_builder(document, provider)   # attach block boxes to pages
line_builder(document, provider)     # decide native-vs-OCR, queue boxes
if not disable_ocr:
    ocr_builder(document, provider)  # run surya recognition
```

`LineBuilder` is the key decision point. For each page it asks four questions:

- provider returned any lines at all?
- ocr_error model says text is "good"?
- layout coverage ≥ 25% of layout blocks intersect provider lines?
- provider lines don't overflow the page bbox or self-intersect heavily?

If all four pass, the page uses pdftext output. Otherwise the page is marked `text_extraction_method = "surya"` and the layout boxes are handed to `OcrBuilder`. See [marker/builders/line.py](../../repos-folder/marker/marker/builders/line.py).

After builders, `StructureBuilder` ([marker/builders/structure.py](../../repos-folder/marker/marker/builders/structure.py)) merges figures/tables with adjacent captions (within 5% of page height) into `FigureGroup`/`TableGroup`/`PictureGroup`, and contiguous list items into `ListGroup`. Ungrouped list items are demoted to plain `Text`.

Then `default_processors` runs.

Finally the renderer is constructed via `resolve_dependencies` and called. The renderer walks `document.render()` which recurses through page → block → child rendering HTML, then transforms HTML to the target format.
