# Content-Type Detection

`fitz_content_types.go` sniffs an input byte slice and returns a MIME-like string that MuPDF accepts as a `magic` argument to `fz_open_document_with_stream`. This is what powers `NewFromMemory` and `NewFromReader`.

The entry point is `contentType(b []byte) string`. An empty string means "unknown" - the constructor will then return `ErrOpenMemory`.

## Detection order

The check runs as a single `switch` with length gates, so cheaper checks run before more expensive ones. The order matters because some signatures are short and could otherwise collide with later, more specific tests.

Length gate `< 8`:

- `image/x-portable-arbitrarymap` (PAM, `P7\n`)
- `image/x-portable-bitmap` (PBM, `P1\n` or `P4\n`)
- `image/x-portable-floatmap` (PFM, `PF\n` or `Pf\n`)
- `image/x-portable-greymap` (PGM, `P2\n` or `P5\n`)
- `image/x-portable-pixmap` (PPM, `P3\n` or `P6\n`)
- `image/gif` (GIF, `GIF8`)

Length gate `< 16`:

- `image/bmp` (BMP, `BM`)
- `image/x-jb2` (JBIG2 with full 8-byte header)

Length gate `< 32`:

- `image/tiff` (TIFF, `II*\0` or `MM\0*`)
- `image/svg+xml` (SVG - parser scans past whitespace, `<?xml`/`<!--` comments, looking for `<svg`)

Length gate `< 64`:

- `image/jpeg` (`FF D8 FF`)
- `image/png` (`89 50 4E 47 0D 0A 1A 0A`)
- `image/jp2` (JPEG 2000, two possible signatures)
- `image/vnd.ms-photo` (JPEG XR, `II BC`)
- `application/pdf` (`%PDF`)
- `image/vnd.adobe.photoshop` (`8BPS`)
- `application/zip` and the four ZIP-based subtypes via `isZIP` plus further checks
- `text/xml` (catches FB2 ebooks, which MuPDF will treat as FB2)
- `application/x-mobipocket-ebook` (MOBI - looks for `BOOKMOBI` or `TEXtREAd` at offset 60)

## ZIP-based formats

When the magic bytes are `PK\x03\x04`, `contentType` looks deeper:

- `isEPUB` - check that bytes 30-57 contain the literal `mimetypeapplication/epub+zip`. This works because the EPUB spec requires the `mimetype` file to be the first uncompressed entry in the archive.
- `isXPS` - check that bytes 30-48 contain `[Content_Types].xml`. MS Office and XPS archives are required to put this file first.
- `isDOCX`, `isXLSX`, `isPPTX` - call `msooxml(buf)` which walks ZIP local file headers (third and possibly fourth) to find a `word/`, `xl/`, or `ppt/` subdirectory entry. Returns the matching MIME type; otherwise returns `application/zip` (treated by fitz as a Comic Book Archive).

The OOXML detection accounts for a 520-byte "extra field" that some writers insert between local file headers, and falls back to scanning a 6000-byte window with the PK signature.

## Tests

`fitz_content_types_test.go` exercises each detection helper with the matching `testdata/test.*` fixture. There is one fixture per format the sniffer handles. Note `testdata/test.cbz` is detected as `application/zip` because the sniffer does not pierce into the archive looking for image content.
