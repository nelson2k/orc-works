# xmpbox

Module: [xmpbox/](../../repos-folder/pdfbox/xmpbox/). Coordinates: `org.apache.pdfbox:xmpbox`.

Standalone Java library for reading and writing XMP (Extensible Metadata Platform, ISO 16684) metadata. PDF stores XMP as an XML stream inside the document catalog; PDFBox uses xmpbox to parse and emit it.

## Subpackages ([org.apache.xmpbox](../../repos-folder/pdfbox/xmpbox/src/main/java/org/apache/xmpbox/))

| Package | Purpose |
|---|---|
| (root) | `XMPMetadata`, `DateConverter`, `XmpConstants` |
| `schema` | Strongly-typed schema classes: `DublinCoreSchema`, `XMPBasicSchema`, `PDFASchema`, `PDFAIdentificationSchema`, `PhotoshopSchema`, etc. |
| `type` | XMP value types: `TextType`, `IntegerType`, `BooleanType`, `URIType`, `DateType`, `AgentNameType`, language alternatives, structured types |
| `xml` | XML parser (`DomXmpParser`) and serializer (`XmpSerializer`) |

## Reading XMP from a PDF

```java
import org.apache.pdfbox.Loader;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.common.PDMetadata;
import org.apache.xmpbox.xml.DomXmpParser;
import org.apache.xmpbox.XMPMetadata;
import org.apache.xmpbox.schema.DublinCoreSchema;

try (PDDocument doc = Loader.loadPDF(new File("in.pdf"))) {
    PDMetadata metadata = doc.getDocumentCatalog().getMetadata();
    if (metadata != null) {
        XMPMetadata xmp = new DomXmpParser().parse(metadata.toByteArray());
        DublinCoreSchema dc = xmp.getDublinCoreSchema();
        String title   = (dc != null) ? dc.getTitle()       : null;
        List<String> a = (dc != null) ? dc.getCreators()    : null;
    }
}
```

## Writing XMP into a PDF

```java
import org.apache.xmpbox.xml.XmpSerializer;
import org.apache.xmpbox.schema.AdobePDFSchema;
import org.apache.xmpbox.schema.XMPBasicSchema;
import java.io.ByteArrayOutputStream;
import java.util.Calendar;

XMPMetadata xmp = XMPMetadata.createXMPMetadata();

XMPBasicSchema basic = xmp.createAndAddXMPBasicSchema();
basic.setCreatorTool("MyApp 1.0");
basic.setCreateDate(Calendar.getInstance());
basic.setModifyDate(Calendar.getInstance());

DublinCoreSchema dc = xmp.createAndAddDublinCoreSchema();
dc.setTitle("Report Q1");
dc.addCreator("Alice");

ByteArrayOutputStream out = new ByteArrayOutputStream();
new XmpSerializer().serialize(xmp, out, /* withXMPMetaTag */ true);
PDMetadata metadata = new PDMetadata(doc);
metadata.importXMPMetadata(out.toByteArray());
doc.getDocumentCatalog().setMetadata(metadata);
```

## Built-in schemas

In `xmpbox.schema`:

- `XMPBasicSchema` — Creator tool, Identifier, Label, Rating, Title, Nickname, MetadataDate, ModifyDate, CreateDate
- `DublinCoreSchema` — Title, Creator(s), Description, Subject(s), Publisher, Date, Rights, Format, Language, ...
- `AdobePDFSchema` — Keywords, PDFVersion, Producer
- `PDFAIdentificationSchema` — Part, Conformance (for PDF/A-1, 2, 3, ...)
- `PhotoshopSchema` — Headline, Credit, Source, AuthorsPosition, City, State, Country
- `XMPMediaManagementSchema`, `XMPRightsManagementSchema`
- `XMPathSchema` / `PDFASchema` for advanced cases

A schema instance is registered when you call `createAndAdd<Foo>Schema()`. Field accessors are typed.

## Value types

`type/` defines the primitive XMP wrappers:

- `TextType`, `BooleanType`, `IntegerType`, `RealType`, `DateType`, `URLType`, `URIType`, `AgentNameType`
- `ProperNameType`, `ChoiceType`, `MIMEType`
- `LangAlt` — language alternative arrays
- `ArrayProperty` — bag / seq / alt
- `ComplexPropertyContainer` — structured type
- Schema-specific complex types like `JobType`, `ThumbnailType`, `LayerType`

## Date handling

`DateConverter` converts between Java `Calendar` and the W3CDTF strings XMP requires (`2026-04-18T12:34:56-07:00`). Most schema setters take `Calendar` directly.

## Parser / serializer

[xml/DomXmpParser.java](../../repos-folder/pdfbox/xmpbox/src/main/java/org/apache/xmpbox/xml/DomXmpParser.java) parses XMP via a JAXP-based DOM. It validates property qualifiers, language alternatives, and namespace declarations.

[xml/XmpSerializer.java](../../repos-folder/pdfbox/xmpbox/src/main/java/org/apache/xmpbox/xml/XmpSerializer.java) emits XMP as RDF/XML. `serialize(xmp, out, withXMPMetaTag)` writes a complete packet wrapped in `<x:xmpmeta>...</x:xmpmeta>` when the last argument is `true`.

## Use independently

`xmpbox.jar` has no dependency on the PDF parser. You can parse / build XMP packets standalone for any host file format (XMP appears in JPEG, TIFF, PSD, etc.).
