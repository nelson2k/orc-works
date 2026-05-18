# Overview

go-fitz is a Go wrapper around the MuPDF fitz library. It can extract pages from PDF, EPUB, MOBI, DOCX, XLSX, PPTX, CBZ, XPS, FB2, and several raster image formats, and render them as raster images, plain text, HTML, or SVG.

## Import path

```go
import "github.com/gen2brain/go-fitz"
```

The package name is `fitz` (not `go-fitz`).

## Supported input formats

Detected via magic-byte sniffing in `fitz_content_types.go`:

- Documents: PDF, EPUB, MOBI, XPS / OXPS, FB2, CBZ (zip), DOCX, XLSX, PPTX.
- Raster images: BMP, GIF, JPEG, JPEG 2000, JPEG XR, PNG, TIFF, PSD, JBIG2, PAM, PBM, PFM, PGM, PPM.
- Vector: SVG.

When opening from a file path, MuPDF picks the handler from the extension. When opening from memory, go-fitz sniffs the bytes first and passes a MIME-like `magic` string to `fz_open_document_with_stream`.

## Output capabilities

For each loaded page:

- Raster image (`*image.RGBA` or PNG bytes) at a chosen DPI.
- Plain text.
- HTML (with optional document header/trailer).
- SVG (text-as-path).
- Page bounding box.
- Link list (URIs only).

Document-level:

- Page count.
- Outline / table of contents (hierarchical).
- Standard metadata fields.

## Bundled vs external library

go-fitz ships pre-built static MuPDF libraries for nine platforms (Linux x86_64 / arm64 / musl variants, Windows x86_64 / arm64, macOS x86_64 / arm64, Android arm64). The default `cgo` build links one of these statically — no system MuPDF needed.

The `extlib` tag switches to linking the system MuPDF (`-lmupdf`). The `nocgo` tag switches to a purego implementation that `dlopen`s `libmupdf.so` / `libmupdf.dll` / `libmupdf.dylib` at runtime.

## Notes from upstream README

- Bundled libraries are built without CJK fonts. CJK requires the external library path.
- Calling methods like `Image()` or `Text()` concurrently on the same `*Document` is not supported. Methods take an internal mutex per document, so they serialize.
- The purego path requires `libffi` and `libmupdf` shared libraries at runtime, and `fitz.FzVersion` (or `FZ_VERSION` env) must match the shared-library version exactly.
