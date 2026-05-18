# Text extraction and rendering

## Text extraction

Package: [org.apache.pdfbox.text](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/text/)

### `PDFTextStripper`

The default text extractor. Reads page content streams, decodes glyphs through their fonts (CMap or Unicode CMap), and emits a string per page.

```java
import org.apache.pdfbox.Loader;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.text.PDFTextStripper;

try (PDDocument doc = Loader.loadPDF(new File("in.pdf"))) {
    PDFTextStripper stripper = new PDFTextStripper();
    stripper.setSortByPosition(true);    // re-sort glyphs to reading order
    stripper.setStartPage(1);
    stripper.setEndPage(3);
    String text = stripper.getText(doc);
}
```

Knobs:

- `setSortByPosition(boolean)` — re-sort glyph runs by Y then X. Use for two-column layouts or when the content stream isn't ordered.
- `setStartPage(int)` / `setEndPage(int)` — page range (1-based).
- `setLineSeparator(String)` / `setParagraphStart(String)` / `setParagraphEnd(String)` — control output formatting.
- `setShouldSeparateByBeads(boolean)` — respect article threads (PDF1.1+ article bead order).
- `setWordSeparator(String)` — what to insert between words (default " ").
- `setSuppressDuplicateOverlappingText(boolean)` — drop near-duplicate glyphs (common in "fake bold" PDFs that draw the same letter twice with offset).

### Variants

- `PDFText2HTML` — emits a minimal HTML representation, preserving page breaks and basic font emphasis.
- `PDFText2Markdown` — same idea, Markdown output (used by `pdfbox-app fromtext` / `export:text -md`).
- `PDFTextStripperByArea` — extract text from one or more rectangles per page.

```java
PDFTextStripperByArea stripper = new PDFTextStripperByArea();
stripper.setSortByPosition(true);
stripper.addRegion("header", new Rectangle2D.Float(0, 0, 612, 72));
stripper.addRegion("body",   new Rectangle2D.Float(0, 72, 612, 720));
stripper.extractRegions(page);
String header = stripper.getTextForRegion("header");
String body   = stripper.getTextForRegion("body");
```

### `TextPosition` and `LegacyPDFStreamEngine`

For fine-grained access (per-glyph positions, exact fonts), subclass `PDFTextStripper` and override `writeString(String, List<TextPosition>)`. Each `TextPosition` carries:

- `getUnicode()` / `getCharacter()` — the character(s) it decoded to
- `getX()`, `getY()` — user-space coordinates
- `getXDirAdj()`, `getYDirAdj()` — display-adjusted (post-rotation)
- `getWidth()`, `getHeight()`, `getFontSize()`, `getFontSizeInPt()`
- `getFont()` — the active `PDFont`

`TextPositionComparator` provides the canonical reading-order ordering used internally when `setSortByPosition(true)`.

### Mark-up extraction

`PDFMarkedContentExtractor` reads the Marked Content operators (`BMC`, `BDC`, `EMC`) and exposes a tree of marked content roots — useful for tagged-PDF accessibility and structured extraction.

## Rendering

Package: [org.apache.pdfbox.rendering](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/rendering/)

### `PDFRenderer`

Rasterizes pages to AWT `BufferedImage`:

```java
import org.apache.pdfbox.rendering.PDFRenderer;
import org.apache.pdfbox.rendering.ImageType;

try (PDDocument doc = Loader.loadPDF(new File("in.pdf"))) {
    PDFRenderer renderer = new PDFRenderer(doc);
    BufferedImage img = renderer.renderImageWithDPI(0, 300, ImageType.RGB);
    ImageIO.write(img, "png", new File("page0.png"));
}
```

| Method | Notes |
|---|---|
| `renderImage(pageIndex)` | At default scale (1.0 = 72 DPI) |
| `renderImage(pageIndex, scale)` | Scale relative to PDF user space |
| `renderImageWithDPI(pageIndex, dpi)` | Scale = dpi/72 |
| `renderImage(pageIndex, scale, imageType)` | Pick pixel format |
| `renderImage(pageIndex, scale, imageType, RenderDestination)` | `EXPORT` (publication) vs `VIEW` (screen) profiles |

`ImageType` enum: `BINARY`, `GRAY`, `RGB`, `ARGB`.

`RenderDestination`: `EXPORT` ignores screen-only optimizations (forces print profile, doesn't anti-alias for screen); `VIEW` (default) tunes for on-screen viewing.

Hooks for subclasses:

- `renderPageToGraphics(pageIndex, graphics)` — draw to an arbitrary `Graphics2D`
- `subsamplingAllowed` flag — let the renderer subsample raster images for speed
- `setRenderingHints(RenderingHints)` — apply Java2D hints

### `PageDrawer`

`PageDrawer` is the actual content-stream engine that renders to Graphics2D. It's a subclass of `PDFGraphicsStreamEngine`. You rarely instantiate it directly; subclass it to intercept specific operators (e.g. catch image-paint events and skip them, or stamp a custom watermark while rendering).

`PageDrawerParameters` (passed to the constructor) controls subsampling, annotation rendering, and the destination type.

### Helpers used internally

- `GlyphCache` — caches rasterized glyphs across pages to amortize font work
- `GroupGraphics` — wraps Graphics2D for transparency groups / soft masks
- `SoftMask` — alpha/luminosity soft-mask handling
- `TilingPaint` — Pattern tiling

### Printing

Package: [org.apache.pdfbox.printing](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/printing/)

```java
PDDocument doc = ...;
PrinterJob job = PrinterJob.getPrinterJob();
job.setPageable(new PDFPageable(doc));
if (job.printDialog()) job.print();
```

`PDFPageable` implements `java.awt.print.Pageable` so the JDK's standard print API handles dialogs, paper size, and the actual job submission. `PDFPrintable` exists for the `Printable` interface variant.
