# fontbox

Module: [fontbox/](../../repos-folder/pdfbox/fontbox/). Coordinates: `org.apache.pdfbox:fontbox`.

A standalone library (used by core PDFBox, but usable on its own) for reading font files of the formats PDF embeds: TrueType, OpenType, CFF, Type 1, AFM, PFB, plus CMap.

## Subpackages ([org.apache.fontbox](../../repos-folder/pdfbox/fontbox/src/main/java/org/apache/fontbox/))

| Package | What it parses |
|---|---|
| `ttf` | TrueType (sfnt — also covers OTF and embedded TTC) |
| `cff` | Compact Font Format (the binary glyph table inside OpenType CFF and PDF Type 1C / CIDFontType0) |
| `type1` | Adobe Type 1 (PostScript), parsed from PFB/PFA |
| `pfb` | Type 1 PFB segment parser |
| `afm` | Adobe Font Metrics (text metrics file accompanying Type 1) |
| `cmap` | PDF CMap files (text-to-cid mapping, plus Adobe public CMaps) |
| `encoding` | PDF/PostScript glyph-name encodings: `MacRoman`, `WinAnsi`, `Standard`, `Symbol`, `ZapfDingbats` |
| `util` | Bounding-box helpers, font matrix conversion |

## Core interfaces

```java
public interface FontBoxFont {
    String getName();
    BoundingBox getFontBBox();
    List<Number> getFontMatrix();
    float getWidth(int code) throws IOException;
    boolean hasGlyph(String name) throws IOException;
    GeneralPath getPath(String name) throws IOException;
}
```

Implemented by `TrueTypeFont`, `Type1Font`, `CFFFont` (and its `CFFType1Font`/`CFFCIDFont` subclasses).

## TrueType / OpenType

```java
import org.apache.fontbox.ttf.TTFParser;
import org.apache.fontbox.ttf.TrueTypeFont;

try (InputStream in = new FileInputStream("Inter.ttf")) {
    TrueTypeFont ttf = new TTFParser().parse(new RandomAccessReadBuffer(in));
    NamingTable name      = ttf.getNaming();
    HeaderTable head      = ttf.getHeader();
    CmapTable  cmap       = ttf.getCmap();
    GlyphTable glyphs     = ttf.getGlyph();
    float width           = ttf.getAdvanceWidth(glyphId);
    GeneralPath path      = ttf.getPath(glyphName);
}
```

Variants:

- `TTFParser` — reads a single TTF.
- `OTFParser` — reads OpenType (CFF or TT outlines).
- `TrueTypeCollection` — parses TTC and gives access to each face.

The classes for individual sfnt tables live alongside (`HorizontalHeaderTable`, `OS2WindowsMetricsTable`, `KerningTable`, `MaximumProfileTable`, `IndexToLocationTable`, `PostScriptTable`, ...).

## CFF

```java
import org.apache.fontbox.cff.CFFParser;
import org.apache.fontbox.cff.CFFFont;

List<CFFFont> fonts = new CFFParser().parse(cffBytes);
```

Each `CFFFont` is either `CFFType1Font` (single set of charstrings) or `CFFCIDFont` (CID-keyed). PDF embeds CFF inside `FontFile3` streams with subtype `Type1C` or `CIDFontType0C`.

## Type 1 (PostScript)

```java
import org.apache.fontbox.type1.Type1Font;
import org.apache.fontbox.pfb.PfbParser;

Type1Font font = Type1Font.createWithPFB(new FileInputStream("font.pfb"));
```

`pfb.PfbParser` splits the three-segment PFB structure (ASCII / binary / ASCII) for the Type 1 parser.

`afm.AFMParser` reads the accompanying `.afm` files for widths and kerning.

## Encodings

`encoding/` provides glyph-name → character mappings for the standard PostScript encodings, plus the `GlyphList` baseline (Adobe AGLFN). These are consumed by PDFBox when reverse-mapping glyph IDs to Unicode.

## CMap

```java
import org.apache.fontbox.cmap.CMapParser;
import org.apache.fontbox.cmap.CMap;

CMap cmap = new CMapParser().parse(inputStream);
String unicode = cmap.toUnicode(/* cid */ 1234);
int cid        = cmap.toCID(/* code */ 0x40);
```

CMap files map character codes to CIDs and vice versa. Used heavily by Type 0 fonts. `CMapManager` (in core `pdmodel/font`) caches them.

## Use independently

Because `fontbox` doesn't import any PDF-specific code, you can pull just `fontbox.jar` to parse fonts in non-PDF contexts (e.g. extracting metrics, validating embedding licenses, exporting glyph shapes as SVG).
