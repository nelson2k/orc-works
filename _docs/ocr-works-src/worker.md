# worker.py

Long-lived Python subprocess that the Go GUI drives over stdin/stdout
JSON. Handles two jobs: render a PDF page to PNG (cheap, via PyMuPDF)
and run OCR on a page (expensive, via marker-pdf).

## Lifecycle

`main()` ([worker.py:244](../../src/worker.py#L244)) loops over stdin,
parses each line as a JSON command, and dispatches:

- `render` → [worker.py:80](../../src/worker.py#L80)
- `ocr`    → [worker.py:162](../../src/worker.py#L162)
- `quit`   → returns from main

Every reply is one JSON object per line written by `send()`
([worker.py:19](../../src/worker.py#L19)). Writes are guarded by
`_send_lock` so progress events emitted from background threads (tqdm)
don't interleave with the main reply.

Errors are caught at the dispatch level and reported as
`{"type":"error","message":..., "traceback":...}` rather than killing
the process.

## tqdm patch (lines 30–71)

Before importing marker/surya, the module monkey-patches `tqdm.tqdm`
and `tqdm.auto.tqdm` with `_EventTqdm`. Every bar instantiation,
update, and close emits a `progress` event instead of writing to a
terminal (the underlying file is forced to `os.devnull`). This is how
model-load and inference progress shows up in the GUI status bar.

## render(path, page, dpi)

Opens the PDF with `pymupdf`, rasterizes the requested page at the
given DPI, returns base64 PNG plus the page count. Bounds-checked: a
bad page index returns `{"type":"error", ...}` instead of raising.

## ocr(path, page)

Drives marker's pipeline by hand instead of calling
`PdfConverter.__call__`, so each stage can emit a stage event:

1. `loading_models` — lazy one-time `create_model_dict()` call. Cached
   in module-global `_marker_models` ([worker.py:101](../../src/worker.py#L101)).
2. `init_converter` — `PdfConverter(artifact_dict=..., config={"page_range":[page]})`
3. `open_pdf` — provider from `provider_from_filepath`
4. `rasterize` — `DocumentBuilder` builds the in-memory document
5. `layout` — layout builder runs, then `_emit_layout_overlay` paints
   colored bboxes per block type and emits an `image` progress event
   that the GUI swaps onto the canvas.
6. `line_detection` — `LineBuilder`
7. `ocr_recognition` — `OcrBuilder` (skipped if `doc_builder.disable_ocr`)
8. `structure` — `StructureBuilder`
9. `processor:<ClassName>` — one event per processor in
   `converter.processor_list`
10. `render` — `converter.renderer`
11. `saving` — markdown + images written by `_save_page_output`

Reply: `{"type":"text", "page":..., "text":..., "saved_to":<dir>}`.

### _emit_layout_overlay

[worker.py:131](../../src/worker.py#L131). Takes the highres page
image, scales each block's bbox into image coords, draws a 4-px
outline colored by block type (see `_BLOCK_COLORS` at
[worker.py:110](../../src/worker.py#L110)), and emits the result as a
`progress` event with `kind:"image"`. Failures are swallowed and
reported as a `overlay_failed` stage event — the OCR run still
completes.

### _save_page_output

[worker.py:227](../../src/worker.py#L227). Writes
`output/<pdf-stem>/page_<NNNN>.md` (1-based, zero-padded) plus any
extracted images, sanitizing the stem with `_FS_UNSAFE`
(`[<>:"/\\|?*]` → `_`). The output dir path is returned to the GUI so
the status bar can show it.

## Notes

- `OUTPUT_ROOT` is `<repo>/output/` — resolved from `__file__`, so it
  doesn't depend on the cwd the GUI happened to launch with.
- Models load on the first OCR request, not at startup. The first page
  is noticeably slower than subsequent ones.
- The script never closes `_devnull`; it lives for the life of the
  process.
