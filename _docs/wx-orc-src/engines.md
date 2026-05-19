# Engines

Five engines, picked from the toolbar dropdown
([main.cpp:50-56](../../wx-ocr-src/src/main.cpp#L50-L56)):

```cpp
{"Auto",             "auto"},
{"Marker",           "marker"},
{"Marker + LLM",     "marker_llm"},
{"VLM (Qwen2.5-VL)", "vlm"},
{"MinerU",           "mineru"},
```

The engine string lands in `ocr()` in worker.py
([worker.py:867-906](../../wx-ocr-src/worker.py#L867-L906)).

## Auto

[`ocr(engine="auto")`](../../wx-ocr-src/worker.py#L878-L894) runs a
small decision tree:

1. **Digital fast-path.** If the page's PyMuPDF `get_text("text")` length
   is ≥ 80 chars
   ([`_has_digital_text`](../../wx-ocr-src/worker.py#L334-L342)),
   skip OCR entirely and run [`ocr_digital`](../../wx-ocr-src/worker.py#L345-L366)
   (pymupdf4llm → markdown). Zero VRAM.
2. **Otherwise run Marker.** [`ocr_marker`](../../wx-ocr-src/worker.py#L383-L464).
3. **Quality gate.** Surya's `OCRErrorPredictor` from the marker model
   dict labels the output good/bad
   ([`_marker_text_is_bad`](../../wx-ocr-src/worker.py#L823-L843)).
   Also: any output shorter than `_MIN_USABLE_TEXT_CHARS` (20) counts
   as bad.
4. **Fallback to VLM** if Marker is "bad" *and* VLM is available locally
   (the model dir exists — [`_vlm_available`](../../wx-ocr-src/worker.py#L819-L820)).
   The result is tagged `engine: "marker→vlm"` and gets a
   `fallback_reason` field.
5. **No VLM available** + bad marker → return Marker's text anyway,
   stamped with `quality_flag: "bad"`.

## Marker / Marker + LLM

[`ocr_marker(path, page, use_llm=False)`](../../wx-ocr-src/worker.py#L383-L464)
walks Marker's pipeline step by step (rasterize → layout → line detect
→ OCR → structure → processors → render), emitting a stage event per
step plus a `kind:"image"` layout-overlay PNG after layout.

With `use_llm=True` the converter wires in an OpenAI-compatible LLM
service (defaults to `http://localhost:8080/v1`, `model=local`, no API
key — i.e. talking to a local llama-server)
([`_marker_llm_config`](../../wx-ocr-src/worker.py#L371-L380)).
Override via `MARKER_LLM_BASE_URL`, `MARKER_LLM_MODEL`,
`MARKER_LLM_API_KEY`.

Marker holds `_marker_models` resident across requests. [`_ensure_marker`](../../wx-ocr-src/worker.py#L265-L272)
unloads the VLM first to free VRAM.

## VLM

[`ocr_vlm`](../../wx-ocr-src/worker.py#L755-L758) dispatches between
two implementations — see [vlm.md](vlm.md).

## MinerU

[`ocr_mineru`](../../wx-ocr-src/worker.py#L767-L809):

- Unloads Marker and VLM first.
- Loads the PDF bytes once, writes them into a temp dir, calls MinerU's
  `do_parse(start_page_id=page, end_page_id=page, backend="pipeline",
  parse_method="auto", formula_enable=True, table_enable=True)`.
- MinerU writes a `<stem>/.../<stem>.md`; we read the first match and
  return its text.
- MinerU caches its own model singleton in-process; we don't track it
  explicitly.

## Saving output

Every successful OCR/digital extract writes to
`<repo>/output/<sanitized-pdf-stem>/page_NNNN.md` via
[`_save_page_output`](../../wx-ocr-src/worker.py#L909-L923). Marker also
saves any extracted images (figures, etc.) alongside.

The `.md` file is also the cache key — see [gotchas.md](gotchas.md).
