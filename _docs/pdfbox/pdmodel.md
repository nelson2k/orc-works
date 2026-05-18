# PDModel layer

Package: [org.apache.pdfbox.pdmodel](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/)

The "PD" layer is the typed object model that wraps the raw COS dictionaries. App code primarily lives here.

## Documents

`PDDocument` — a parsed (or newly created) PDF.

```java
import org.apache.pdfbox.Loader;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.PDPage;
import org.apache.pdfbox.pdmodel.PDPageContentStream;
import org.apache.pdfbox.pdmodel.font.PDType1Font;
import org.apache.pdfbox.pdmodel.font.Standard14Fonts;

// Load
try (PDDocument doc = Loader.loadPDF(new File("in.pdf"))) {
    int pageCount = doc.getNumberOfPages();
    PDPage page0 = doc.getPage(0);
}

// Create
try (PDDocument doc = new PDDocument()) {
    PDPage page = new PDPage();
    doc.addPage(page);
    try (PDPageContentStream cs = new PDPageContentStream(doc, page)) {
        cs.beginText();
        cs.setFont(new PDType1Font(Standard14Fonts.FontName.HELVETICA), 12);
        cs.newLineAtOffset(72, 720);
        cs.showText("Hello, PDF.");
        cs.endText();
    }
    doc.save("out.pdf");
}
```

Key methods on `PDDocument`:

- `getNumberOfPages()`, `getPage(i)`, `getPages()` → `PDPageTree` (iterable)
- `addPage(PDPage)`, `removePage(int)`, `importPage(PDPage)` (deep-copy from another doc)
- `getDocumentCatalog()` → `PDDocumentCatalog` (page tree, outline, names, viewer prefs)
- `getDocumentInformation()` → `PDDocumentInformation` (Title, Author, Subject, ...)
- `getVersion()` / `setVersion(float)`
- `save(file|stream|path)` / `saveIncremental(stream)`
- `protect(ProtectionPolicy)` / `getEncryption()`
- `isEncrypted()`, `getCurrentAccessPermission()`
- `close()` — releases the underlying `RandomAccessRead`

## Pages

`PDPage` — a single page.

```java
PDPage page = doc.getPage(0);
PDRectangle mediaBox  = page.getMediaBox();
PDRectangle cropBox   = page.getCropBox();
PDResources resources = page.getResources();
int rotation = page.getRotation();
```

Use `PDPageContentStream` to write content; constructor variants control append vs replace, compression, and the explicit "save graphics state" toggle:

```java
new PDPageContentStream(doc, page);
new PDPageContentStream(doc, page, AppendMode.APPEND, /* compress */ true);
new PDPageContentStream(doc, page, AppendMode.PREPEND, true, /* resetContext */ true);
```

Operators on `PDPageContentStream` cover text, paths, color, transformations, images, clipping — see [PDAbstractContentStream](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/PDAbstractContentStream.java) and its subclasses for the full API.

## Resources

`PDResources` is the per-page dictionary of fonts, images, color spaces, graphics states, patterns, shadings, and XObjects. Access:

```java
for (COSName key : resources.getFontNames())    { PDFont font = resources.getFont(key); }
for (COSName key : resources.getXObjectNames()) { PDXObject xo = resources.getXObject(key); }
for (COSName key : resources.getColorSpaceNames()) { PDColorSpace cs = resources.getColorSpace(key); }
```

`PDResources` is shared by reference between pages that reference the same resource dictionary.

## Fonts

[pdmodel/font/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/font/)

| Class | Purpose |
|---|---|
| `PDFont` | Abstract base (decoding code → unicode + advance widths) |
| `PDType1Font` | Type 1 / Standard 14 — `new PDType1Font(Standard14Fonts.FontName.HELVETICA_BOLD)` |
| `PDTrueTypeFont` | TrueType, embedded or not |
| `PDType0Font` | Composite (CID) — `PDType0Font.load(doc, ttfFile)` |
| `PDCIDFont` / `PDCIDFontType0` / `PDCIDFontType2` | Descendant CID fonts |
| `PDType3Font` | Type 3 (glyphs as content streams) |
| `PDFontDescriptor` | Font metrics + embedded program reference |

Standard 14 PostScript fonts ship with the library — they're the fonts every PDF reader is required to provide. Use `Standard14Fonts.FontName` enum.

Embed TrueType/OTF:

```java
PDType0Font font = PDType0Font.load(doc, new File("Inter-Regular.ttf"));
```

Font discovery in the OS goes through `FontMappers` and the cached `FileSystemFontProvider`. Override the provider via `FontMappers.set(...)`.

## Images

[pdmodel/graphics/image/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/graphics/image/)

```java
PDImageXObject img = PDImageXObject.createFromFile("photo.jpg", doc);
contentStream.drawImage(img, 72, 72, 200, 200);   // x, y, w, h in PDF units (1 unit = 1/72 inch)
```

Helpers:

- `PDImageXObject.createFromFile(path, doc)` — auto-detect JPEG / PNG / TIFF / etc.
- `PDImageXObject.createFromByteArray(doc, bytes, name)`
- `JPEGFactory.createFromImage(doc, BufferedImage)` — write as DCT-encoded JPEG
- `LosslessFactory.createFromImage(doc, BufferedImage)` — write as Flate-encoded raster
- `CCITTFactory.createFromImage(doc, BufferedImage)` — write as 1-bit fax-encoded

## Forms (AcroForm)

[pdmodel/interactive/form/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/interactive/form/)

```java
PDAcroForm form = doc.getDocumentCatalog().getAcroForm();
PDField field = form.getField("FirstName");
field.setValue("Alice");
form.flatten();   // burn appearances into pages, drop interactivity
```

Field subtypes: `PDTextField`, `PDCheckBox`, `PDRadioButton`, `PDComboBox`, `PDListBox`, `PDPushButton`, `PDSignatureField`.

## Annotations

[pdmodel/interactive/annotation/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/interactive/annotation/)

Subclasses cover the Acrobat annotation taxonomy: `PDAnnotationText`, `PDAnnotationFreeText`, `PDAnnotationLink`, `PDAnnotationHighlight`, `PDAnnotationStamp`, `PDAnnotationFileAttachment`, ... List per page via `page.getAnnotations()`.

## Encryption

[pdmodel/encryption/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/encryption/)

```java
// Password protection
StandardProtectionPolicy spp = new StandardProtectionPolicy("ownerPW", "userPW", new AccessPermission());
spp.setEncryptionKeyLength(256);
doc.protect(spp);

// Certificate-based
PublicKeyProtectionPolicy pkpp = new PublicKeyProtectionPolicy();
pkpp.addRecipient(recipientPolicy);
doc.protect(pkpp);
```

`AccessPermission` controls "can print", "can extract text", etc.

## Digital signatures

[pdmodel/interactive/digitalsignature/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel/interactive/digitalsignature/)

```java
PDSignature sig = new PDSignature();
sig.setFilter(PDSignature.FILTER_ADOBE_PPKLITE);
sig.setSubFilter(PDSignature.SUBFILTER_ADBE_PKCS7_DETACHED);
sig.setName("Alice"); sig.setSignDate(Calendar.getInstance());
doc.addSignature(sig, signatureInterface, signatureOptions);
doc.saveIncremental(outputStream);
```

The actual cryptographic signing is delegated to a user-supplied `SignatureInterface` (often backed by `bcpkix-jdk18on` from Bouncy Castle, which is what `pdfbox-app` bundles).

## Document catalog

`PDDocumentCatalog` exposes top-level structures:

- `getPages()` → `PDPageTree`
- `getDocumentOutline()` / `setDocumentOutline(PDDocumentOutline)`
- `getNames()` → `PDDocumentNameDictionary` (destinations, JavaScript, embedded files)
- `getViewerPreferences()`
- `getOpenAction()`
- `getAcroForm()` / `setAcroForm(PDAcroForm)`
- `getMetadata()` (XMP)
- `getStructureTreeRoot()` (tagged-PDF accessibility)

## Multi-page operations

[pdmodel/multipdf is actually `multipdf/`](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/multipdf/) — concatenation, splitting, overlays, layer manipulation:

- `PDFMergerUtility` — `merger.addSource(file); merger.setDestinationFileName(...); merger.mergeDocuments(...);`
- `Splitter` — `splitter.setSplitAtPage(n); List<PDDocument> parts = splitter.split(doc);`
- `Overlay` — composite a "watermark" PDF over each page of the input
- `LayerUtility` — import an arbitrary page as a Form XObject for re-use as a watermark/overlay
- `PageExtractor` — copy a page range out into a new document
