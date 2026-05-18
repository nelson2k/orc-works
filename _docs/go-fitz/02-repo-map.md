# Repo Map

Source inspected at `repos-folder/go-fitz`.

## Top level

- `README.md` - overview, build tags, basic example.
- `AUTHORS`, `COPYING` - authors and AGPL license.
- `go.mod`, `go.sum` - module `github.com/gen2brain/go-fitz`; deps: `ebitengine/purego`, `jupiterrider/ffi`, `golang.org/x/sys`.

## Go source files

Each file has a `//go:build ...` constraint and the set selected at compile time depends on `cgo` and the user-supplied tags.

- `fitz.go` - package doc, error sentinels, `MaxStore`, `FzVersion`, `Outline`, `Link` types, the `bytePtrToString` helper. Always compiled.
- `fitz_cgo.go` - the cgo implementation. Defines `Document`, all constructors, all rendering / text / metadata / outline methods. Selected when `cgo && !nocgo`.
- `fitz_cgo_cgo.go` - cgo `LDFLAGS` for every supported `GOOS/GOARCH/musl` combination. Tells the linker which bundled `.a` file under `libs/` to use. Selected when `cgo && !nocgo && !extlib`.
- `fitz_cgo_extlib.go` - cgo `LDFLAGS` for linking against a system-installed MuPDF (`-lmupdf -lm`). Selected when `cgo && !nocgo && extlib && !pkgconfig`.
- `fitz_cgo_extlib_pkgconfig.go` - same idea but via `pkg-config`. Selected when `cgo && !nocgo && extlib && pkgconfig`.
- `fitz_nocgo.go` - purego implementation. Same API as the cgo version, but every MuPDF call goes through `purego.Dlsym` / `ffi.Call`. Selected when `!cgo || nocgo`.
- `fitz_content_types.go` - magic-byte sniffer that returns a MIME-type string for a byte slice. Used by `NewFromMemory`.
- `fitz_vendor.go` - blank-import side effect to keep the bundled `include/` and `libs/` directories in `go mod vendor` output. Selected by `//go:build required`.
- `purego_darwin.go`, `purego_linux.go`, `purego_windows.go` - per-OS `loadLibrary()` and `procAddress()` helpers for the purego implementation.

## Test files

- `example_test.go` - `ExampleNew` - end-to-end example: open `test.pdf`, write each page as `.jpg`, `.txt`, `.html`, and `.svg`.
- `fitz_test.go` - `TestImage`, `TestImageFromMemory`, `TestLinks`, `TestText`, `TestHTML`, `TestPNG`, `TestSVG`, `TestToC`, `TestMetadata`, `TestBound`, `TestEmptyBytes`. Each test reads `testdata/test.pdf` and writes outputs into a temp directory.
- `fitz_content_types_test.go` - per-format content-type detection tests using the `testdata/test.*` fixtures.

## Subdirectories

- `include/` - bundled MuPDF C headers used by the cgo build.
  - `mupdf/fitz.h`, `mupdf/memento.h`.
  - `mupdf/fitz/` - per-area headers: `archive.h`, `band-writer.h`, `bidi.h`, `bitmap.h`, `buffer.h`, `color.h`, `compress.h`, `compressed-buffer.h`, `config.h`, `context.h`, `crypt.h`, `device.h`, `display-list.h`, `document.h`, `export.h`, `filter.h`, `font.h`, `geometry.h`, `getopt.h`, `glyph-cache.h`, `glyph.h`, `hash.h`, `heap-imp.h`, `heap.h`, `image.h`, `link.h`, `log.h`, `outline.h`, `output-svg.h`, `output.h`, `path.h`, `pixmap.h`, `pool.h`, `separation.h`, `shade.h`, `store.h`, `story-writer.h`, `story.h`, `stream.h`, `string-util.h`, `structured-text.h`, `system.h`, `text.h`, `track-usage.h`, `transition.h`, `tree.h`, `types.h`, `util.h`, `version.h`.
  - `vendor.go` - blank-import marker so `go mod vendor` keeps the headers.

- `libs/` - bundled prebuilt static MuPDF libraries:
  - `libmupdf_<os>_<arch>[_musl].a` and `libmupdfthird_<os>_<arch>[_musl].a` for: `android_arm64`, `darwin_amd64`, `darwin_arm64`, `linux_amd64`, `linux_amd64_musl`, `linux_arm64`, `linux_arm64_musl`, `windows_amd64`, `windows_arm64`.
  - `vendor.go` - blank-import marker so `go mod vendor` keeps the libraries.

- `testdata/` - one file per supported format used by the test suite: `test.bmp`, `test.cbz`, `test.docx`, `test.epub`, `test.fb2`, `test.gif`, `test.jb2`, `test.jp2`, `test.jpg`, `test.jxr`, `test.mobi`, `test.pam`, `test.pbm`, `test.pdf`, `test.pfm`, `test.pgm`, `test.png`, `test.ppm`, `test.pptx`, `test.psd`, `test.svg`, `test.tif`, `test.xlsx`, `test.xps`.
