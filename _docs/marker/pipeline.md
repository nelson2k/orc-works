# Pipeline

Implemented by `PdfConverter.build_document` (and overridden in `TableConverter`,
`OCRConverter`, `ExtractionConverter`).

## 1. Pick a provider

`provider_from_filepath(filepath)` (in `providers/registry.py`) sniffs the file
with `filetype` and falls back to extension matching:

| Type     | Provider               |
|----------|------------------------|
| PDF      | `PdfProvider`          |
| Image    | `ImageProvider`        |
| DOCX     | `DocumentProvider`     |
| XLSX     | `SpreadSheetProvider`  |
| PPTX     | `PowerPointProvider`   |
| EPUB     | `EpubProvider`         |
| HTML     | `HTMLProvider`         |

## 2. Build the Document

`DocumentBuilder(config)(provider, layout_builder, line_builder, ocr_builder)`:

1. Create `PageGroup`s with low-res (96 DPI) and high-res (192 DPI) images.
2. `LayoutBuilder` runs surya layout → assigns block types to regions on each
   page (or uses `force_layout_block` to skip detection).
3. `LineBuilder` runs surya detection + OCR-error model, merges detected lines
   with PDF-provider lines, decides per-page whether to OCR.
4. `OcrBuilder` runs surya recognition on the regions flagged for OCR; spans
   are reconstructed from per-character HTML-like tags (`<math>`, formatting,
   `<br>`).

## 3. Group structure

`StructureBuilder` runs once per `Document`:

- Groups figures / tables / pictures with adjacent `Caption` / `Footnote` blocks
  into `*Group` blocks.
- Groups consecutive `ListItem`s into `ListGroup`s.
- Demotes lonely `ListItem`s back to `Text`.

## 4. Apply processors

The converter holds an ordered `processor_list` (default in
`PdfConverter.default_processors`). LLM-simple processors are batched through a
single `LLMSimpleBlockMetaProcessor` so that all per-block prompts go out in
parallel — see [processors_llm.md](processors_llm.md).

## 5. Render

The renderer is resolved from `--output_format` (or passed directly). It walks
the `Document` tree producing one of: markdown, html, json, chunks, ocr_json,
or extraction output. See [renderers.md](renderers.md).
