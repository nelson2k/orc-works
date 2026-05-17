# Overview

PDF Font Extractor is a **single-PDF, single-output-ZIP** Swing app. Drop in
a PDF, click Run, get a `<timestamp>_extractedFonts.zip` containing one
`.ttf` per embedded TrueType font found in the document (de-duplicated by
filename).

It is not a converter, an OCR tool, or a layout extractor. The PDF text
content is untouched; only the binary font streams are pulled out.

## Source layout

Tiny — just two Java files:

| File                              | Job                                                            |
|-----------------------------------|----------------------------------------------------------------|
| `src/app/Main.java`               | The whole Swing GUI + font-extraction logic in one class.      |
| `src/app/SettingsManager.java`    | UI constants: colours, fonts, borders, base64 app icon.        |
| `PdfFontExtractor.jar`            | Pre-built fat JAR (PDFBox + FlatLaf + this code).              |
| `sample/sample.pdf`               | Example input.                                                 |
| `sample/<timestamp>_extractedFonts/` | Example output (19 TTFs from sample.pdf).                   |

## When you'd use it

- Reverse-engineering a corporate template: pull out the in-house font so you
  can match it.
- Reproducing a PDF in a different tool that needs the same font installed.
- Forensics — seeing exactly which font files were shipped inside a document.
- Building a font corpus from a folder of PDFs (though you'd need to script
  around the GUI for batch use, or wrap PDFBox yourself).

## When you wouldn't

- You want characters, words, or layout → use [pdftext](../pdftext/README.md)
  or [marker](../marker/README.md).
- You want the font *names* only → PDFBox's `PDResources.getFontNames()` or
  pypdf's `Page.extractText()` metadata is enough.
- The PDF embeds **subsets** of fonts (it almost always does) — see
  [limitations.md](limitations.md) about the `ABCDEE+` prefix glyphs only.
- Type 1, CFF (CID Type 0), or OpenType-with-CFF fonts → not exported by
  this tool, see [extraction.md](extraction.md).

## License + provenance

- License: see `repos-folder/pdf-font-extractor/LICENSE`.
- Author: incubated-geek-cc on GitHub. Related blog post:
  https://geek-cc.medium.com/how-to-extract-embedded-fonts-from-a-pdf-as-valid-font-files-in-java-1d202fa06f4e
- Built with Apache PDFBox 3.0.0 (Apache 2.0) + FlatLaf 3.x (Apache 2.0).
