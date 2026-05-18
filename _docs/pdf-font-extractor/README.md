# PDF Font Extractor

A Java 8 desktop app that reads a PDF and writes out every embedded TrueType font as a standalone `.ttf` inside a single `.zip` archive. Built on Apache PDFBox 3.0 and FlatLaf for the look-and-feel.

## What it does

For each `PDPage`, walks the page resources, finds `PDFontDescriptor` entries that carry a `FontFile2` (TrueType) stream, and writes that stream to a ZIP entry named `<fontName>.ttf`. Handles both direct TrueType fonts (`PDTrueTypeFont`) and Type 0 fonts whose descendant is a CIDFontType2 (`PDType0Font` → `PDCIDFontType2`). Also recurses into `PDFormXObject` resources so fonts buried in Form XObjects are picked up too.

Duplicate font names are deduplicated against a `HashMap<String, Boolean>` so the same face isn't written twice.

## Project layout

```
pdf-font-extractor/
├── README.md
├── LICENSE                       MIT
├── PdfFontExtractor.jar          Pre-built fat JAR
├── img/                          README screenshots (cover, logo, gui, demo)
├── sample/                       Example outputs + sample.pdf
└── src/
    └── app/
        ├── Main.java             Swing UI + extraction logic
        └── SettingsManager.java  Theming / colors / fonts / default dir
```

This is a flat Maven-less project; the prebuilt fat JAR at the repo root (`PdfFontExtractor.jar`) bundles PDFBox + FontBox + FlatLaf + commons-logging.

## Running

```bash
java -jar PdfFontExtractor.jar
```

Or with an installed JDK ≥ 1.8.

The GUI:

1. **Choose File** — opens a `JFileChooser` filtered to `*.pdf`. The first page is rendered at 300 DPI by `PDFRenderer` and shown as a 250 × 250 thumbnail. File name + page count are displayed.
2. **Run ≫** — kicks off a `SwingWorker` that opens the PDF via `Loader.loadPDF(file)`, walks each page's resources, builds the ZIP in memory, then opens a save dialog for the destination `.zip`.
3. **Reset All** — clears state and disables Run.

The output ZIP is `yyyyMMdd_hhmmss_extractedFonts.zip` by default; the OS file manager opens it on success.

## How extraction works ([Main.java:480](../../repos-folder/pdf-font-extractor/src/app/Main.java))

```java
processResources(PDResources resources) {
    for (COSName key : resources.getFontNames()) {
        PDFont font = resources.getFont(key);
        PDFontDescriptor fd = null;
        String name = null;

        if (font instanceof PDTrueTypeFont) {
            fd = font.getFontDescriptor();
            name = font.getName();
        } else if (font instanceof PDType0Font) {
            PDCIDFont desc = ((PDType0Font) font).getDescendantFont();
            if (desc instanceof PDCIDFontType2) {
                fd = desc.getFontDescriptor();
                name = font.getName();
            }
        }

        if (fd != null) {
            PDStream ff2 = fd.getFontFile2();   // FontFile2 = TrueType bytes
            if (ff2 != null && !FONTNAME_MAPPER.containsKey(name + ".ttf")) {
                writeZipEntry(name + ".ttf", ff2.toByteArray());
                FONTNAME_MAPPER.put(name + ".ttf", true);
            }
        }
    }
    // recurse into Form XObjects
    for (COSName xname : resources.getXObjectNames()) {
        PDXObject xo = resources.getXObject(xname);
        if (xo instanceof PDFormXObject) {
            processResources(((PDFormXObject) xo).getResources());
        }
    }
}
```

Only fonts with a `FontFile2` stream (TrueType data) are exported. Type 1 (`FontFile`) and Type 0/CFF (`FontFile3`) are ignored. Fonts that aren't embedded at all (only referenced by name) produce no output.

## SettingsManager ([SettingsManager.java](../../repos-folder/pdf-font-extractor/src/app/SettingsManager.java))

Holds the visual constants:

- Color palette (`WHITE`, `LIGHT`, `GRAY`, `MEDIUM_GRAY`, `GRAY_DARK`, `DARK`)
- Border presets (`EMPTY_BORDER`, `PANEL_PADDING_BORDER`, `RAISED_PANEL_MEDIUM_GRAY_BORDER`, lowered variants)
- Plain/bold/italic font factories
- A `POINTER_CURSOR` (hand) constant
- The base64-encoded `APP_ICON_URI` (PNG) used for the window icon
- `DEFAULT_DIR` (system user home, updated on each file dialog use)

`Main`'s top-level `static SettingsManager SETTINGS_MGR` is shared across the GUI.

## Look and feel

Applies FlatLaf (`FlatLightLaf`) on startup and overrides a few UIManager keys:

- `Component.arrowType: triangle`
- `ScrollBar.showButtons: true`
- `Button.arc: 5`
- `Spinner.buttonArrowColor`
- Font and color overrides for Button, Label, OptionPane, TextField, ScrollPane

The window is non-resizable (`setResizable(false)`), 725 × 485, centered on screen, exits on close.

## Threading

All heavy lifting (file dialogs, PDF loading, font extraction) runs inside `SwingWorker<Boolean, ...>` subclasses so the EDT stays responsive. UI updates happen in `done()` / `process()` after worker completion.

## Dependencies

Bundled inside `PdfFontExtractor.jar`:

| Library | Purpose |
|---|---|
| Apache PDFBox 3.0.0 | PDF parsing, font extraction, page rendering |
| Apache PDFBox `multipdf.Splitter` | Get first page for thumbnail |
| Apache FontBox | Font format handling (transitive) |
| FlatLaf | Modern Swing look-and-feel |
| Apache Commons Logging | Used by PDFBox |

JDK 1.8 is the documented target; nothing in the code uses post-8 features.

## Limitations

- Only fonts embedded as TrueType (FontFile2). PostScript / CFF fonts are skipped.
- Subset-tagged fonts (PDF embedding often subsets a font to just the glyphs used in the document) export only the subset — the result is technically a valid TTF but only contains the glyphs that appeared in the PDF.
- No CLI — the JAR is GUI-only.
