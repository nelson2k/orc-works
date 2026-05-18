# API

The public API is identical between the cgo and purego builds; only the implementation differs. Signatures below match both `fitz_cgo.go` and `fitz_nocgo.go`.

## Types

### `Document`

The handle for a loaded document. Holds the MuPDF context, the document pointer, and an internal `sync.Mutex` that serializes per-document calls.

```go
type Document struct {
    // unexported: ctx, data, doc, mtx, stream
}
```

### `Outline`

```go
type Outline struct {
    Level int     // hierarchy level, starting at 1
    Title string
    URI   string
    Page  int     // page number for internal links
    Top   float64 // top coordinate
}
```

### `Link`

```go
type Link struct {
    URI string
}
```

Only the URI is exposed; coordinates are not currently surfaced.

## Package variables

- `MaxStore` - resource store size in bytes before MuPDF starts evicting cached fonts and images. Default `256 << 20` (256 MiB).
- `FzVersion` - shared-library version for the purego build. Must match the runtime `libmupdf` exactly. Can also be supplied via the `FZ_VERSION` environment variable. Default `"1.24.9"`.

## Errors

All sentinel errors:

`ErrNoSuchFile`, `ErrCreateContext`, `ErrOpenDocument`, `ErrEmptyBytes`, `ErrOpenMemory`, `ErrLoadPage`, `ErrRunPageContents`, `ErrPageMissing`, `ErrCreatePixmap`, `ErrPixmapSamples`, `ErrNeedsPassword`, `ErrLoadOutline`.

## Constructors

```go
fitz.New(filename string) (*Document, error)
fitz.NewFromMemory(b []byte)  (*Document, error)
fitz.NewFromReader(r io.Reader) (*Document, error)
```

- `New` resolves the filename to an absolute path and requires the file to exist (`ErrNoSuchFile` otherwise).
- `NewFromMemory` calls `contentType` to sniff a magic string, then `fz_open_document_with_stream`. Empty input → `ErrEmptyBytes`.
- `NewFromReader` reads everything via `io.ReadAll` and delegates to `NewFromMemory`.
- All three return `ErrNeedsPassword` if the document is encrypted - no password API is provided.

## Per-page methods

```go
func (f *Document) NumPage() int
func (f *Document) Image(pageNumber int) (*image.RGBA, error)
func (f *Document) ImageDPI(pageNumber int, dpi float64) (*image.RGBA, error)
func (f *Document) ImagePNG(pageNumber int, dpi float64) ([]byte, error)
func (f *Document) Text(pageNumber int) (string, error)
func (f *Document) HTML(pageNumber int, header bool) (string, error)
func (f *Document) SVG(pageNumber int) (string, error)
func (f *Document) Links(pageNumber int) ([]Link, error)
func (f *Document) Bound(pageNumber int) (image.Rectangle, error)
```

Notes:

- `Image` is a convenience wrapper that calls `ImageDPI(page, 300.0)`. There is no zero/default-DPI overload at lower resolution.
- `ImageDPI` and `ImagePNG` build a fresh pixmap per call - no caching. They scale the page bounds by `dpi/72`, fill the pixmap with `0xff` (white) before drawing, and disable MuPDF's display-list cache via `FZ_NO_CACHE`.
- `Text` runs the page through an `stext` device with `flags = 0`. Output is plain text.
- `HTML` runs the page through an `stext` device with `FZ_STEXT_PRESERVE_IMAGES`. When `header` is true the output is wrapped in `<html>...</html>` via MuPDF's `fz_print_stext_header_as_html` / `fz_print_stext_trailer_as_html`.
- `SVG` uses `fz_new_svg_device` with `FZ_SVG_TEXT_AS_PATH` (no text glyphs, only path geometry).
- `Links` walks the linked list returned by `fz_load_links` and copies the URI of each entry. Coordinates are not extracted.
- `Bound` returns the page rectangle in unscaled page units (PDF points for PDF, document-native units otherwise) as an `image.Rectangle` with `image.Point` coordinates cast from `float`.
- Every per-page method calls `f.mtx.Lock()` first, so concurrent calls on the same `*Document` serialize.
- Page indices are zero-based; passing `pageNumber >= NumPage()` returns `ErrPageMissing`.

## Document-level methods

```go
func (f *Document) ToC() ([]Outline, error)
func (f *Document) Metadata() map[string]string
func (f *Document) Close() error
```

- `ToC` walks the outline recursively via `fz_load_outline` and flattens it into `[]Outline` with `Level` reflecting depth. Returns `ErrLoadOutline` if MuPDF has no outline for the document.
- `Metadata` looks up ten fixed keys via `fz_lookup_metadata` and returns them as a `map[string]string`: `format`, `encryption`, `title`, `author`, `subject`, `keywords`, `creator`, `producer`, `creationDate`, `modDate`. Values are 256-byte buffer reads (truncation possible for very long fields). The buffer is not trimmed - trailing NULs may appear.
- `Close` drops the document, the in-memory stream (if any), and the MuPDF context, then nils the held byte slice. No error is returned in practice; the signature exists for `defer doc.Close()` patterns.

## What is not exposed

Notable MuPDF capabilities that go-fitz does not surface:

- Annotation reading / writing.
- Form fields / widgets.
- Redaction.
- PDF editing, save, merge, split, page deletion.
- OCR (MuPDF integrates Tesseract internally, but go-fitz does not expose those entry points).
- Password-protected documents (no `Authenticate` method).
- Per-page transform matrices beyond uniform DPI scaling.
- Link coordinates / quads.
- Tabular data extraction.
