# PDF Font Extractor — Docs Index

A small Java Swing GUI tool that extracts every embedded TrueType font from a
PDF and packs them into a ZIP of `.ttf` files. Built on Apache PDFBox 3.0.0,
Java 1.8.

Source repo: `repos-folder/pdf-font-extractor/`.

Not related to text/layout extraction — this only pulls out the **font
files** themselves so you can reuse them outside the PDF.

## How it works at a glance

```
PDF file (selected via JFileChooser)
  └─ PDFBox: Loader.loadPDF(file) → PDDocument
      └─ for each PDPage:
          └─ processResources(page.getResources())
              ├─ for each font in resources.getFontNames():
              │    if PDTrueTypeFont OR (PDType0Font wrapping PDCIDFontType2):
              │      → pull FontFile2 stream from FontDescriptor
              │      → write to a ZipEntry named <fontName>.ttf
              └─ recurse into PDFormXObject resources (nested forms)
  └─ write ZIP to user-chosen path
```

## Pages

- [overview.md](overview.md) — what it does, when you'd want it
- [install.md](install.md) — running the JAR, building from source
- [usage.md](usage.md) — GUI walkthrough
- [extraction.md](extraction.md) — PDFBox calls used, font type handling
- [limitations.md](limitations.md) — subset prefixes, CFF/Type1, licensing
- [vs_others.md](vs_others.md) — vs fontTools, pdfminer, pikepdf, mutool
