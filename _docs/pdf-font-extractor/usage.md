# Usage (GUI walkthrough)

The whole app is one window with three buttons. Implemented in
[`Main.java`](repos-folder/pdf-font-extractor/src/app/Main.java).

## 1. Choose File

Click **📂 Choose File** → standard `JFileChooser` filtered to `*.pdf`.

What happens behind the scenes (`chooseFileAction`):

- `Loader.loadPDF(selectedFile)` → `PDDocument`.
- `document.getNumberOfPages()` → displayed in the page-count label.
- `Splitter().split(document).get(0)` → grab the first page as a separate
  document.
- `PDFRenderer.renderImageWithDPI(0, 300, ImageType.RGB)` → render that
  first page at 300 DPI for the thumbnail.
- Thumbnail is scaled to 250×250 preserving aspect ratio and shown in the
  preview panel.
- An "UPLOAD SUCCESS" `JOptionPane` confirms the file is ready.

## 2. Run

Click **Run ≫** (disabled until a file is loaded).

What happens (`extractEmbeddedFontsAction`):

- Build a `<yyyyMMdd_hhmmss>_extractedFonts.zip` filename in memory.
- Create a `ZipOutputStream` over a `ByteArrayOutputStream`.
- `Loader.loadPDF(selectedFile)` again (independent of the load done for
  the preview — see [extraction.md](extraction.md)).
- For each `PDPage`, call `processResources(page.getResources())` to walk
  fonts (and recurse into Form XObject resources).
- On any matching font, append a `ZipEntry` named `<fontName>.ttf` and copy
  the TTF byte stream from PDFBox's `FontDescriptor.getFontFile2()`.
- A save-dialog (`JFileChooser` save mode, ZIP filter) lets you pick the
  output path. The default name is the timestamped one.
- The ZIP bytes are flushed to disk and `Desktop.getDesktop().open(zip)` is
  called — Windows shells will show the contents; macOS will mount it.

## 3. Reset All

Click **Reset All** to clear the filename/page labels, thumbnail, and
in-memory state (`FONTNAME_MAPPER`, the upload details map, the absolute
path). The next "Choose File" starts clean.

## Visual conventions

- App font: FlatLaf Light look-and-feel, Consolas 11/12 for labels.
- Page count is displayed in **monospace Unicode digits** (`𝟶..𝟿`), via
  `convertToMonospace()`. Purely cosmetic.

## What's *not* exposed

- Output format choice (always ZIP of `.ttf`).
- Page subset (whole document is scanned).
- Font subset filter (everything matching the criteria is exported).
- CLI / scripting / drag-drop / multi-file selection.

If you want any of those, you have to fork or wrap PDFBox yourself; see
[vs_others.md](vs_others.md) for libraries that make this easy from Python.
