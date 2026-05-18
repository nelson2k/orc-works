# Apache PDFBox

The [Apache PDFBox](https://pdfbox.apache.org/) library is a Java toolkit for working with PDF documents — reading, writing, modifying, extracting text and images, rendering pages, filling forms, handling encryption, etc. Snapshot version in the source tree: `4.0.0-SNAPSHOT`. Build: Java 11+, Maven 3.

## Multi-module Maven project

The reactor pom in [pom.xml](../../repos-folder/pdfbox/pom.xml) declares:

```
parent          Common Maven settings, dependency versions, plugin config
io              Random-access I/O primitives (no PDF logic)
fontbox         Font format readers (TTF, CFF, Type 1, AFM, CMap, PFB)
xmpbox          XMP metadata (ISO 16684) read/write
pdfbox          Core PDF library: cos, pdfparser, pdmodel, rendering, text, multipdf
debugger        Swing PDF debugger UI
debugger-app    Bundled debugger executable
tools           Command-line utilities (export:text, render, merge, split, ...)
app             Standalone runnable assembly (pdfbox-app fat JAR)
examples        Example programs
```

## Top-level layout

```
pdfbox/
├── pom.xml                       Reactor POM (lists all modules)
├── parent/pom.xml                Shared dependency/plugin versions
├── assembly.xml                  Assembly descriptor for the fat JAR
├── pdfbox-checkstyle-5.xml       Checkstyle rules
├── pdfbox-eclipse-formatter.xml  Eclipse code style
├── suppressions.xml              Checkstyle suppressions
├── README.md / RELEASE-NOTES.txt / LICENSE.txt / NOTICE.txt / KEYS
├── benchmark/                    Microbenchmarks (JMH)
│
├── io/                           "pdfbox-io"
├── fontbox/                      "fontbox"
├── xmpbox/                       "xmpbox"
├── pdfbox/                       "pdfbox"  — the bulk of the library
├── debugger/                     "pdfbox-debugger"
├── debugger-app/                 "pdfbox-debugger-app"
├── tools/                        "pdfbox-tools"  — picocli commands
├── app/                          "pdfbox-app"  — fat-JAR assembly
└── examples/                     "pdfbox-examples"
```

## Coordinates

```
groupId    = org.apache.pdfbox
artifactId = pdfbox          (or pdfbox-tools, pdfbox-app, pdfbox-io, fontbox, xmpbox, pdfbox-debugger)
version    = 4.0.0-SNAPSHOT  (or the latest released 3.x for stable use)
license    = Apache License 2.0
java       = 11+
```

## The library, in three layers

1. **COS layer** ([pdfbox/cos/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/cos)) — the raw PDF object model. `COSDocument`, `COSDictionary`, `COSArray`, `COSStream`, `COSName`, `COSString`, `COSInteger`, `COSFloat`, `COSBoolean`, `COSNull`. This is what the parser produces.
2. **PD layer** ([pdfbox/pdmodel/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdmodel)) — typed wrappers over COS. `PDDocument`, `PDPage`, `PDPageTree`, `PDResources`, `PDFont`, `PDImage*`, `PDAnnotation`, `PDAcroForm`, etc. This is the public API most apps use.
3. **High-level operations** — [`text/`](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/text), [`rendering/`](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/rendering), [`printing/`](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/printing), [`multipdf/`](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/multipdf): `PDFTextStripper`, `PDFRenderer`, `PDFPageable`, `PDFMergerUtility`, `Splitter`, `LayerUtility`, `Overlay`, `PageExtractor`.

The supporting infrastructure: parsers ([pdfparser/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdfparser)), writers ([pdfwriter/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/pdfwriter)), content-stream engine ([contentstream/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/contentstream)), filter codecs ([filter/](../../repos-folder/pdfbox/pdfbox/src/main/java/org/apache/pdfbox/filter)).

## Entry points

```java
import org.apache.pdfbox.Loader;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.text.PDFTextStripper;
import org.apache.pdfbox.rendering.PDFRenderer;

// Load
try (PDDocument doc = Loader.loadPDF(new File("input.pdf"))) {

    // Extract text
    String text = new PDFTextStripper().getText(doc);

    // Render page 0 at 300 DPI
    BufferedImage img = new PDFRenderer(doc).renderImageWithDPI(0, 300);
}
```

`org.apache.pdfbox.Loader` is the entry point for reading PDFs from `File`, `byte[]`, `InputStream`, or `RandomAccessRead`. All `Loader.loadPDF(...)` overloads return `PDDocument`.

## Standalone CLI

`pdfbox-app` assembles all `pdfbox-tools` commands into a single runnable JAR:

```bash
java -jar pdfbox-app-4.0.0-SNAPSHOT.jar [command] [options]

# Examples
java -jar pdfbox-app.jar export:text in.pdf out.txt
java -jar pdfbox-app.jar render -dpi 150 in.pdf
java -jar pdfbox-app.jar merge a.pdf b.pdf c.pdf out.pdf
java -jar pdfbox-app.jar split -split 2 in.pdf
java -jar pdfbox-app.jar debug                          # opens the Swing debugger
```

