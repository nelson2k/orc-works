# marker — notes

Source: `repos-folder/marker` (datalab-to / VikParuchuri). Version `1.10.2`. License: GPL-3.0 code, OpenRAIL-M weights.

What it is: a pipeline that turns PDF / image / DOCX / PPTX / XLSX / EPUB / HTML into markdown, HTML, JSON, or chunks. Optionally calls an LLM for higher accuracy on tables, forms, math, headers, complex regions.

Pipeline at a glance:

1. `Provider` reads the source file and exposes pages + native text/lines + images.
2. `DocumentBuilder` constructs the empty `Document` of `PageGroup`s.
3. `LayoutBuilder` runs surya layout, attaches typed blocks to each page.
4. `LineBuilder` decides per-page whether native text is good enough; otherwise queues surya OCR boxes.
5. `OcrBuilder` runs surya recognition on the queued boxes.
6. `StructureBuilder` groups captions with figures/tables and groups list items.
7. `Processors` clean each block kind (tables, equations, code, headers, references, debug…). LLM processors slot in here when `use_llm` is set.
8. `Renderer` walks the block tree and produces the chosen output format.
