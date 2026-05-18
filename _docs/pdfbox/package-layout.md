# Package layout

Contents of `repos-folder/pdfbox/`:

```
pdfbox/
├── pom.xml                       Reactor POM (lists submodules: parent, io, fontbox,
│                                   xmpbox, pdfbox, debugger, tools, app, debugger-app, examples)
├── parent/pom.xml                Shared Maven settings (Java 11+, dependency versions, plugins)
├── assembly.xml                  Fat-JAR assembly descriptor (used by pdfbox-app)
├── pdfbox-checkstyle-5.xml       Checkstyle rules
├── pdfbox-eclipse-formatter.xml  Eclipse code style
├── suppressions.xml              Checkstyle suppressions
├── README.md / LICENSE.txt / NOTICE.txt / KEYS / RELEASE-NOTES.txt
│
├── io/                           "pdfbox-io" — Random-access I/O primitives
│   └── src/main/java/org/apache/pdfbox/io/
│       ├── IOUtils.java
│       ├── MemoryUsageSetting.java         Off-heap, on-disk, or in-memory scratch buffers
│       ├── RandomAccess.java               Combined read+write interface
│       ├── RandomAccessRead.java           Random-access read interface (parser input)
│       ├── RandomAccessReadBuffer.java     In-memory byte[]
│       ├── RandomAccessReadBufferedFile.java       Memory-backed File reader
│       ├── RandomAccessReadMemoryMappedFile.java   mmap reader
│       ├── RandomAccessReadView.java       Subrange view
│       ├── SequenceRandomAccessRead.java   Concatenate multiple readers
│       ├── RandomAccessStreamCache.java    Stream caching strategy
│       ├── ScratchFile.java                On-disk scratch space
│       └── NonSeekableRandomAccessReadInputStream.java
│
├── fontbox/                      "fontbox" — Font format readers
│   └── src/main/java/org/apache/fontbox/
│       ├── FontBoxFont.java     EncodedFont.java
│       ├── ttf/                 TrueType, OpenType, TTC parser + sfnt tables
│       ├── cff/                 Compact Font Format parser
│       ├── type1/               Adobe Type 1 (PostScript) parser
│       ├── pfb/                 Type 1 PFB segment parser
│       ├── afm/                 Adobe Font Metrics parser
│       ├── cmap/                CMap parser (PDF + Adobe-public CMaps)
│       ├── encoding/            PS/PDF encodings: MacRoman, WinAnsi, Standard, Symbol, ZapfDingbats
│       └── util/                Bbox + matrix helpers
│
├── xmpbox/                       "xmpbox" — XMP metadata
│   └── src/main/java/org/apache/xmpbox/
│       ├── XMPMetadata.java     DateConverter.java     XmpConstants.java
│       ├── schema/              DublinCoreSchema, XMPBasicSchema, PDFASchema, Photoshop, ...
│       ├── type/                Primitive + complex XMP types
│       └── xml/                 DomXmpParser, XmpSerializer
│
├── pdfbox/                       "pdfbox" — core library
│   ├── pom.xml
│   └── src/main/java/org/apache/pdfbox/
│       ├── Loader.java                Top-level Loader.loadPDF / loadFDF / loadXFDF
│       ├── cos/                       Raw PDF object model (COSDictionary, COSArray, COSStream, ...)
│       ├── pdfparser/                 PDFParser, COSParser, BaseParser, BruteForceParser, FDFParser
│       │                                PDFStreamParser, PDFObjectStreamParser, PDFXRefStream/PDFXrefStreamParser
│       ├── pdfwriter/                 COSWriter — serialize back to PDF
│       ├── pdmodel/                   The typed (PD) object model:
│       │   ├── PDDocument.java, PDPage.java, PDPageTree.java, PDPageContentStream.java
│       │   ├── PDResources.java, PDDocumentCatalog.java, PDDocumentInformation.java
│       │   ├── common/                Box, Rectangle, COSObjectable, COSArrayList
│       │   ├── font/                  PDFont, PDTrueTypeFont, PDType0Font, PDCIDFont*, FontMappers
│       │   ├── graphics/              PDXObject, PDImageXObject, color, image, form, pattern, shading
│       │   ├── encryption/            StandardProtectionPolicy, PublicKeyProtectionPolicy, AccessPermission
│       │   ├── fdf/                   Forms Data Format
│       │   ├── fixup/                 Document repair passes
│       │   ├── documentinterchange/   Tagged PDF accessibility
│       │   └── interactive/           form, annotation, action, digitalsignature,
│       │                                  documentnavigation, pagenavigation, viewerpreferences, measurement
│       ├── contentstream/             PDFStreamEngine, PDFGraphicsStreamEngine, PDContentStream
│       │   └── operator/              One class per PDF operator (BT, m, l, S, Tf, Tj, ...)
│       ├── filter/                    Filter codecs (Flate, ASCII85, ASCIIHex, LZW, CCITT, DCT, JBIG2, Crypt, RunLength, JPEG2000)
│       ├── rendering/                 PDFRenderer, PageDrawer, ImageType, RenderDestination, SoftMask, TilingPaint
│       ├── printing/                  PDFPageable, PDFPrintable (java.awt.print bridges)
│       ├── multipdf/                  PDFMergerUtility, Splitter, Overlay, LayerUtility, PageExtractor
│       ├── text/                      PDFTextStripper, PDFTextStripperByArea, PDFText2HTML, PDFText2Markdown,
│       │                                LegacyPDFStreamEngine, PDFMarkedContentExtractor, TextPosition
│       └── util/                      Internal utilities
│
├── debugger/                     "pdfbox-debugger" — Swing PDF debugger UI
│   └── src/main/java/org/apache/pdfbox/debugger/
│       ├── PDFDebugger.java     Main window
│       ├── colorpane/, flagbitspane/, fontencodingpane/, hexviewer/,
│       │   pagepane/, signaturepane/, streampane/, stringpane/, treestatus/
│
├── debugger-app/                 Standalone debugger runnable JAR
├── tools/                        "pdfbox-tools" — picocli CLI commands
│   └── src/main/java/org/apache/pdfbox/tools/
│       ├── PDFBox.java          Umbrella command
│       ├── Decrypt.java         Encrypt.java
│       ├── WriteDecodedDoc.java (decode)
│       ├── ExtractText.java     ExtractImages.java     ExtractXMP.java
│       ├── ExportFDF.java       ExportXFDF.java        ImportFDF.java   ImportXFDF.java
│       ├── OverlayPDF.java
│       ├── PDFToImage.java      (render)
│       ├── PDFMerger.java       PDFSplit.java
│       ├── ImageToPDF.java      TextToPDF.java
│       ├── PrintPDF.java
│       ├── PDFText2HTML.java    PDFText2Markdown.java
│       ├── Version.java
│       └── imageio/
│
├── app/                          "pdfbox-app" — fat-JAR assembly (depends on tools + bcpkix-jdk18on)
├── examples/                     "pdfbox-examples" — runnable examples per feature area
└── benchmark/                    JMH microbenchmarks
```

## Maven coordinates

```
<dependency>
  <groupId>org.apache.pdfbox</groupId>
  <artifactId>pdfbox</artifactId>
  <version>4.0.0-SNAPSHOT</version>
</dependency>

<!-- The fat JAR with all CLI commands -->
<dependency>
  <groupId>org.apache.pdfbox</groupId>
  <artifactId>pdfbox-app</artifactId>
  <version>4.0.0-SNAPSHOT</version>
</dependency>
```

Other artifact IDs: `pdfbox-io`, `fontbox`, `xmpbox`, `pdfbox-tools`, `pdfbox-debugger`, `pdfbox-debugger-app`, `pdfbox-examples`.

## Build

```bash
mvn clean install            # full reactor build (Java 11+, Maven 3+)
mvn -pl pdfbox install -am   # core only, with dependents
```

## Major third-party dependencies (parent/pom.xml)

| Dep | Purpose |
|---|---|
| `org.bouncycastle:bcpkix-jdk18on` | Cryptography for encryption / signatures (bundled in pdfbox-app) |
| `info.picocli:picocli` | CLI argument parsing for `tools/` |
| `commons-logging` | Used internally for logging |
| `org.apache.pdfbox:jbig2-imageio` (optional) | JBIG2 image filter support |
| `com.github.jai-imageio:jai-imageio-jpeg2000` (optional) | JPEG2000 image filter support |
| `junit-jupiter`, `mockito` | Tests |

## Java compatibility

- Java 11 minimum (4.x branch). Older 2.x / 3.x branches target Java 8.
- AWT is required for `rendering/` and `printing/` — headless servers need `java.awt.headless=true` and risk losing access to AWT-backed APIs like `PDFRenderer` (which still works headless).
