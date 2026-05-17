# Docling Integration Notes

Docling is not currently wired into this app. The current backend uses Marker.

## Why inspect Docling

Docling has a production-style conversion architecture with:

- explicit `DocumentConverter`
- configurable format options
- cached pipelines
- threaded PDF stages
- page-level queues
- timeout handling
- layout visualization helper

Those ideas are useful for improving OCR Works even if the backend remains
Marker-based.

## Useful ideas to borrow

- Use a conversion job model instead of one blocking request.
- Emit page/stage events from the pipeline.
- Keep page images and parsed pages only when needed for debug mode.
- Separate stdout/stderr logs and structured progress events.
- Track partial success at page level.

## For real-time overlays

Docling's threaded stages are easier to instrument than a single monolithic
call. A similar pattern for Marker would be:

```text
job queue
  -> page/stage worker
  -> event stream
  -> frontend overlay
```
