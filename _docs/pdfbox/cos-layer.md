# COS layer

Package: [org.apache.pdfbox.cos](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos/)

The COS layer mirrors the *raw* PDF object model from the ISO 32000 spec. Every typed wrapper in `pdmodel` ultimately backs onto a `COSBase` instance. You drop down to COS when you need to read or write data the typed API doesn't model.

## Object types

| Class | PDF type | Notes |
|---|---|---|
| `COSBase` | abstract | Root of all COS objects |
| `COSBoolean` | bool | Singletons `COSBoolean.TRUE` / `COSBoolean.FALSE` |
| `COSInteger` | integer | Cached small values |
| `COSFloat` | real | `setValue(double)` rounds to PDF-stable text |
| `COSNumber` | abstract | Parent of `COSInteger` and `COSFloat` |
| `COSString` | string | Holds raw bytes; `getString()` decodes via PDFEncoding |
| `COSName` | name | Cached, interned-style (e.g. `COSName.FONT`, `COSName.FILTER`) |
| `COSArray` | array | Ordered `List<COSBase>` |
| `COSDictionary` | dict | `Map<COSName, COSBase>` |
| `COSStream` | stream | A dict with an encoded byte payload |
| `COSObject` | indirect ref | Lazy reference to an object in the xref table |
| `COSNull` | null | Singleton `COSNull.NULL` |
| `COSDocument` | — | The root container, owns the xref table |

## Reading values

```java
COSDictionary dict = page.getCOSObject();
COSName subtype = dict.getCOSName(COSName.SUBTYPE);
int rotation    = dict.getInt(COSName.ROTATE, 0);
COSArray box    = dict.getCOSArray(COSName.MEDIA_BOX);
```

Typed getters honor defaults:

- `getInt(key, default)`, `getFloat(key, default)`, `getBoolean(key, default)`
- `getString(key)`, `getNameAsString(key)`, `getDate(key)`
- `getDictionaryObject(key)` — resolves through indirect refs
- `getItem(key)` — keeps an indirect ref as-is

Iteration:

```java
for (COSName key : dict.keySet()) {
    COSBase value = dict.getDictionaryObject(key);
}
```

## Writing values

```java
dict.setInt(COSName.LENGTH, bytes.length);
dict.setName(COSName.SUBTYPE, "Form");
dict.setItem(COSName.RESOURCES, resourcesDict);
```

Reserved name constants are interned in `COSName` as `public static final`. Use them rather than re-allocating.

## Streams

`COSStream` extends `COSDictionary` and adds an encoded byte payload. Common operations:

```java
COSStream stream = new COSStream();
try (OutputStream out = stream.createOutputStream(COSName.FLATE_DECODE)) {
    out.write(rawBytes);
}

// Read decoded
try (InputStream in = stream.createInputStream()) { ... }

// Read raw (still encoded)
try (InputStream in = stream.createRawInputStream()) { ... }
```

The `Filter` argument to `createOutputStream` decides the encoding chain (`FLATE_DECODE`, `DCT_DECODE`, `LZW_DECODE`, `ASCII85_DECODE`, `RUN_LENGTH_DECODE`, etc.).

## Indirect references

`COSObject` is a lazy pointer into the xref table:

- `getObject()` — dereferences (may trigger lazy parsing)
- `getObjectNumber()`, `getGenerationNumber()`
- Indirect refs participate in object reuse so two parts of the document can share a dictionary

## COSDocument

`COSDocument` is the container — owns the xref table, the cross-reference stream, the trailer, and the underlying `RandomAccessRead`. `PDDocument.getDocument()` returns it.

The trailer dictionary is fetched via `getTrailer()` and holds: `/Root` (catalog), `/Info` (info dict), `/Encrypt`, `/ID`, `/Size`, `/Prev`.

## Update states

[COSDocumentState.java](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos/COSDocumentState.java), [COSUpdateInfo.java](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos/COSUpdateInfo.java), [COSIncrement.java](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos/COSIncrement.java), [COSUpdateState.java](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos/COSUpdateState.java) track per-object dirty bits so `PDDocument.saveIncremental` writes only changed objects.

## Parser entry points

[pdfparser/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdfparser/)

| Parser | Role |
|---|---|
| `PDFParser` | Top-level parser; consumes a `RandomAccessRead`, yields `COSDocument` |
| `COSParser` | Base class, handles xref + trailer |
| `BaseParser` | Token-level parser (numbers, names, strings, hex strings) |
| `BruteForceParser` | Fallback path for PDFs with corrupted xref |
| `PDFStreamParser` | Parses a content stream into operator + operand tokens |
| `PDFObjectStreamParser` | Parses object streams (PDF 1.5+) |
| `FDFParser` | FDF form-data parser |

The high-level `Loader.loadPDF(...)` constructs the right parser.

## Writer

[pdfwriter/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdfwriter/)

`COSWriter` serializes a `COSDocument` back to bytes. `PDDocument.save(...)` wraps it. Incremental update via `COSWriter(stream, signing-state)`.

## Content-stream engine

[contentstream/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/contentstream/)

`PDFStreamEngine` is an abstract interpreter for PDF content streams (the `BT ... ET` text blocks, `m l h S` paths, etc.). Subclasses override `processOperator` to do work per operator:

- `PageDrawer` (in `rendering/`) implements rendering atop AWT `Graphics2D`.
- `LegacyPDFStreamEngine` / `PDFMarkedContentExtractor` (in `text/`) drive text extraction.
- Custom subclasses can do whatever — e.g. extract images at known positions, build a vector dump.

Operator implementations live in [contentstream/operator/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/contentstream/operator/), one class per operator (`ShowText`, `MoveTo`, `CurveTo`, `SetFont`, ...).
