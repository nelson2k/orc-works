# Command-line tools (pdfbox-tools / pdfbox-app)

Module: [tools/](../../repos-folder/pdfbox/tools/). Entry point: [PDFBox.java](../../repos-folder/pdfbox/tools/src/main/java/org/apache/pdfbox/tools/PDFBox.java) — wraps every command in a picocli root command.

The `pdfbox-app` assembly bundles all tool jars into a single runnable JAR. Invocation pattern:

```bash
java -jar pdfbox-app.jar [COMMAND] [OPTIONS]
java -jar pdfbox-app.jar help <command>
```

## Subcommands ([PDFBox.java](../../repos-folder/pdfbox/tools/src/main/java/org/apache/pdfbox/tools/PDFBox.java))

| Subcommand | Class | Purpose |
|---|---|---|
| `debug` | `PDFDebugger` (from `pdfbox-debugger`) | Swing-based PDF inspector (only registered if not headless) |
| `decrypt` | `Decrypt` | Remove password / RC4 / AES encryption |
| `encrypt` | `Encrypt` | Apply password or certificate encryption |
| `decode` | `WriteDecodedDoc` | Rewrite the file with streams decoded |
| `export:images` | `ExtractImages` | Pull every embedded image to disk |
| `export:xmp` | `ExtractXMP` | Dump XMP metadata as XML |
| `export:text` | `ExtractText` | `PDFTextStripper` driver |
| `export:fdf` | `ExportFDF` | Form data → FDF |
| `export:xfdf` | `ExportXFDF` | Form data → XFDF |
| `import:fdf` | `ImportFDF` | Apply FDF data to a PDF |
| `import:xfdf` | `ImportXFDF` | Apply XFDF data |
| `overlay` | `OverlayPDF` | Composite one PDF over another |
| `print` | `PrintPDF` | Send to a printer |
| `render` | `PDFToImage` | Rasterize pages to images (PNG/JPG/TIFF) |
| `merge` | `PDFMerger` | Concatenate multiple PDFs |
| `split` | `PDFSplit` | Split into N-page chunks |
| `fromimage` | `ImageToPDF` | Pack images into a new PDF |
| `fromtext` | `TextToPDF` | Wrap plain text into a PDF |
| `version` | `Version` | Print PDFBox version |
| `help` | `picocli HelpCommand` | Built-in |

## Examples

```bash
# Extract text
java -jar pdfbox-app.jar export:text -encoding UTF-8 in.pdf out.txt
java -jar pdfbox-app.jar export:text -html in.pdf out.html
java -jar pdfbox-app.jar export:text -md  in.pdf out.md     # via PDFText2Markdown
java -jar pdfbox-app.jar export:text -sort -password secret in.pdf out.txt

# Render pages
java -jar pdfbox-app.jar render -format png -dpi 300 in.pdf
java -jar pdfbox-app.jar render -format jpg -quality 0.85 -startPage 1 -endPage 3 in.pdf

# Encrypt / decrypt
java -jar pdfbox-app.jar encrypt -O ownerpw -U userpw -keyLength 256 in.pdf out.pdf
java -jar pdfbox-app.jar decrypt -password secret in.pdf out.pdf

# Forms
java -jar pdfbox-app.jar export:fdf in.pdf data.fdf
java -jar pdfbox-app.jar import:fdf in.pdf data.fdf out.pdf

# Composition
java -jar pdfbox-app.jar merge a.pdf b.pdf c.pdf merged.pdf
java -jar pdfbox-app.jar split -split 1 in.pdf            # 1 page per file
java -jar pdfbox-app.jar overlay watermark.pdf -odd in.pdf -default out.pdf

# Conversion
java -jar pdfbox-app.jar fromimage scan-01.png scan-02.png scans.pdf
java -jar pdfbox-app.jar fromtext -fontfile font.ttf article.txt article.pdf

# Print
java -jar pdfbox-app.jar print -printerName "HP LaserJet" -silentPrint in.pdf

# Image extraction
java -jar pdfbox-app.jar export:images -prefix img -alwaysNew in.pdf

# Decoded copy (useful for debugging)
java -jar pdfbox-app.jar decode in.pdf in_decoded.pdf

# Debugger UI (headless mode hides this command)
java -jar pdfbox-app.jar debug in.pdf
```

## Common options

Most commands share these picocli-defined options:

- `-password <pw>` — password for encrypted input
- `-startPage <n>` / `-endPage <n>` — page range (1-based)
- `-h` / `--help` — print command help
- `-V` / `--version` — print version

Each command class declares its specific options via `@CommandLine.Option`; consult `pdfbox help <cmd>`.

## Suppressing the macOS Dock icon

`PDFBox.main` sets `apple.awt.UIElement=true` before parsing the command line, so non-`debug` subcommands don't bounce the Dock icon on macOS.

## Programmatic use of individual tools

Each tool class has a `public static void main(String[] args)` of its own (e.g. `ExtractText.main(args)`), so you can call them directly without going through the umbrella `PDFBox` class:

```java
org.apache.pdfbox.tools.ExtractText.main(new String[]{"-encoding", "UTF-8", "in.pdf", "out.txt"});
```

## Debugger app

The separate [debugger-app/](../../repos-folder/pdfbox/debugger-app/) module is a stand-alone JAR for just the debugger UI (without the rest of the tools). The Swing UI itself lives in [debugger/](../../repos-folder/pdfbox/debugger/) and has panes for the COS tree, decoded streams, embedded fonts, encodings, hex view, page render, signature inspection, and color profile inspection.
