# Worker JSON protocol

Newline-delimited JSON over stdin (commands) and stdout (replies +
progress events). Bidirectional; one process per GUI session.

## Commands (GUI → worker)

```json
{"cmd":"render", "path":"<abs pdf path>", "page":0, "dpi":120}
{"cmd":"ocr",    "path":"<abs pdf path>", "page":0}
{"cmd":"quit"}
```

`page` is zero-based. Unknown command → `{"type":"error","message":"unknown command: ..."}`.

## Replies (worker → GUI)

A single request produces zero or more `progress` events followed by
exactly one terminal reply.

### Terminal replies

```json
// render success
{"type":"image", "page":0, "pages":42, "png_base64":"..."}

// ocr success
{"type":"text", "page":0, "text":"# Heading\n...", "saved_to":"C:/.../output/foo"}

// any failure
{"type":"error", "message":"<str>", "traceback":"<optional>"}
```

### Progress events (`type:"progress"`)

Three subtypes, distinguished by `kind`:

```json
// pipeline stage marker
{"type":"progress", "kind":"stage", "name":"layout"}

// tqdm bar lifecycle (start | tick | end)
{"type":"progress", "kind":"tqdm", "event":"tick",
 "desc":"Recognizing layout", "n":3, "total":10}

// layout overlay PNG to swap onto the page canvas
{"type":"progress", "kind":"image", "page":0, "png_base64":"..."}
```

Stage names you'll see during an OCR call:
`loading_models`, `init_converter`, `open_pdf`, `rasterize`, `layout`,
`line_detection`, `ocr_recognition`, `structure`,
`processor:<ClassName>` (one per processor), `render`, `saving`,
and the failure-only `overlay_failed`.

## GUI side of the wire

[gui-main.md](gui-main.md) — `Worker.request` writes the JSON,
scans replies, invokes `onProgress` for `type:"progress"`, and returns
the first non-progress reply. The whole exchange is serialized by
`Worker.mu`, so at most one in-flight request at a time.
