# Tests And Examples

## `example_test.go`

A single function `ExampleNew` that:

1. Opens `test.pdf` via `fitz.New`.
2. Creates a temp dir.
3. Iterates pages four times, writing each page as:
   - `.jpg` via `doc.Image(n)` → `jpeg.Encode`.
   - `.txt` via `doc.Text(n)`.
   - `.html` via `doc.HTML(n, true)` (header included).
   - `.svg` via `doc.SVG(n)`.

This doubles as the upstream README example and shows the full output-format surface in one pass.

## `fitz_test.go`

Eleven `Test*` functions exercising the public API. Each one opens `testdata/test.pdf` (or its bytes), writes its outputs into a temp dir (`os.MkdirTemp(... "fitz")`), and cleans up with `defer os.RemoveAll`.

- `TestImage` - `doc.Image(n)` per page, JPEG-encode each.
- `TestImageFromMemory` - `os.ReadFile` + `NewFromMemory`, then same per-page JPEG encode.
- `TestLinks` - read links from page 2.
- `TestText` - `doc.Text(n)` per page.
- `TestHTML` - `doc.HTML(n, true)` per page.
- `TestPNG` - `doc.ImagePNG(n, 300.0)` per page.
- `TestSVG` - `doc.SVG(n)` per page.
- `TestToC` - call `doc.ToC()` once.
- `TestMetadata` - call `doc.Metadata()` once.
- `TestBound` - call `doc.Bound(n)` per page.
- `TestEmptyBytes` - assert `NewFromMemory(nil)` (and an empty-Reader path) returns `ErrEmptyBytes`. Uses a local `emptyReader` type that returns `io.EOF` immediately.

## `fitz_content_types_test.go`

Per-format content-type detection tests covering each of the magic-byte helpers in `fitz_content_types.go`. Uses the `testdata/test.<ext>` fixtures - one per format.

## `testdata/`

Single-page fixtures for every input format the sniffer recognizes:

`test.bmp`, `test.cbz`, `test.docx`, `test.epub`, `test.fb2`, `test.gif`, `test.jb2`, `test.jp2`, `test.jpg`, `test.jxr`, `test.mobi`, `test.pam`, `test.pbm`, `test.pdf`, `test.pfm`, `test.pgm`, `test.png`, `test.ppm`, `test.pptx`, `test.psd`, `test.svg`, `test.tif`, `test.xlsx`, `test.xps`.

## Running the tests

Standard Go invocation:

```
go test ./...
```

To test the purego path locally:

```
go test -tags nocgo ./...
```

This requires a system `libmupdf` whose `FZ_VERSION` matches `fitz.FzVersion` (currently `"1.24.9"`).
