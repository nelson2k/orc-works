# How extraction works

The font-extraction logic is one method: `processResources(PDResources)` in
`Main.java`. It's called once per page (and recursively for nested Form
XObjects). About 35 lines of meaningful code.

## Walk fonts on a resources object

```java
for (COSName key : resources.getFontNames()) {
    PDFont font = resources.getFont(key);
    PDFontDescriptor fd = null;
    String name = null;

    if (font instanceof PDTrueTypeFont) {
        fd = font.getFontDescriptor();
        name = font.getName();
    } else if (font instanceof PDType0Font) {
        PDCIDFont descendantFont = ((PDType0Font) font).getDescendantFont();
        if (descendantFont instanceof PDCIDFontType2) {
            fd = descendantFont.getFontDescriptor();
            name = font.getName();
        }
    }

    if (fd != null) {
        String fontFilename = name + ".ttf";
        PDStream ff2Stream = fd.getFontFile2();
        if (ff2Stream != null && !FONTNAME_MAPPER.containsKey(fontFilename)) {
            byte[] buffer = ff2Stream.toByteArray();
            // write buffer into the ZipOutputStream as a new ZipEntry
            FONTNAME_MAPPER.put(fontFilename, true);
        }
    }
}
```

## Font types it handles

| PDF font class            | Underlying format    | Extracted? | Notes                                              |
|---------------------------|----------------------|-----------:|----------------------------------------------------|
| `PDTrueTypeFont`          | TrueType (TTF)       | ✓          | Pulled via `FontDescriptor.getFontFile2()`         |
| `PDType0Font` + `PDCIDFontType2` | TrueType (CID-keyed) | ✓     | Same call on the descendant font descriptor        |
| `PDType0Font` + `PDCIDFontType0` | CFF / OpenType   | ✗          | Would need `getFontFile3()` (no Type 1C handling)  |
| `PDType1Font`             | Type 1 (PFB)         | ✗          | Would need `getFontFile()` and conversion           |
| `PDType1CFont`            | Compact Font Format  | ✗          | Would need `getFontFile3()`                         |
| `PDType3Font`             | Procedural (PDF ops) | ✗          | Not a font file at all — just drawing commands     |

So **the tool is TTF-only**. Modern PDFs (especially Adobe and LibreOffice
output) lean heavily on Type 0 + CID Type 2, which IS supported. PDFs from
older toolchains or some math-heavy LaTeX builds use Type 1 / Type 1C and
will silently produce empty / partial output.

## Form XObjects (nested resources)

After the font loop:

```java
for (COSName name : resources.getXObjectNames()) {
    PDXObject xobject = resources.getXObject(name);
    if (xobject instanceof PDFormXObject) {
        PDResources formResources = ((PDFormXObject) xobject).getResources();
        processResources(formResources);  // recurse
    }
}
```

This catches fonts used only inside reusable form XObjects (page templates,
header/footer modules, embedded SVGs). Without the recursion you'd miss
fonts that appear only in those nested streams.

## De-duplication

A static `HashMap<String, Boolean> FONTNAME_MAPPER` tracks already-written
filenames per run. Same font name across multiple pages → written once.

This is filename-based dedup. If a PDF embeds two **different** subsets of
the "same" font under the same name string, the second one is dropped.
Rare in practice but possible.

## Output naming

```
<font.getName()> + ".ttf"
```

For subset fonts, `font.getName()` returns the subset-prefixed name PDFBox
exposes — typically `ABCDEE+FontName` or similar 6-letter random prefixes
(per PDF spec). See [limitations.md](limitations.md).

## Why "TTF" might lie

The extracted `.ttf` is whatever bytes were in `FontFile2`. The PDF spec
allows `FontFile2` to contain **OpenType-with-TrueType-outlines** as well as
classic TTF. The file extension is `.ttf` either way and most font editors
(FontForge, FontTools) accept both.
