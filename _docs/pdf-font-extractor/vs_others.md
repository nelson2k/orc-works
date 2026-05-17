# Alternatives

This is a Java Swing app. If you want CLI / batch / Python — there are good
alternatives. All extract the same `FontFile` / `FontFile2` / `FontFile3`
streams; they differ in interface, language, and which font types they
handle.

## Python

| Tool             | Install                | Handles                           | Notes                                              |
|------------------|------------------------|-----------------------------------|----------------------------------------------------|
| **pikepdf**      | `pip install pikepdf`  | TTF, CFF, Type 1                  | Best Python option. Wraps qpdf. Walk `Page.Resources.Font` and dump the stream. |
| **pdfminer.six** | `pip install pdfminer.six` | TTF, CFF (basic)              | More involved API; built for text extraction first. |
| **PyMuPDF**      | `pip install pymupdf`  | TTF, CFF, Type 1, Type 3 (raster) | One-line `doc.extract_font(xref)`. **AGPL** — same license issue marker avoids. |
| **fontTools**    | `pip install fonttools` | Inspect / convert after extraction | Doesn't extract from PDFs; use to validate / convert the TTFs you pulled. |

### pikepdf one-liner (closest equivalent)

```python
import pikepdf

with pikepdf.open("doc.pdf") as pdf:
    seen = set()
    for page in pdf.pages:
        for name, font in (page.Resources.get("/Font") or {}).items():
            ff = font.get("/FontDescriptor", {}).get("/FontFile2")
            if ff is None: continue
            fname = str(font.get("/BaseFont", name))
            if fname in seen: continue
            seen.add(fname)
            with open(f"{fname}.ttf", "wb") as f:
                f.write(ff.read_bytes())
```

20 lines, supports recursion into form XObjects with a tiny extension, runs
on any OS without Java, scriptable.

## Command-line

| Tool         | Install              | Command                                          | Notes                                          |
|--------------|----------------------|--------------------------------------------------|------------------------------------------------|
| **mutool**   | mupdf-tools          | `mutool extract doc.pdf`                          | Extracts fonts AND images to current dir. Fast. |
| **pdf-parser.py** | `pip install pdfid` | `pdf-parser.py -k FontFile2 doc.pdf`         | Forensics-grade, raw object dump.              |
| **pdfimages**| poppler-utils         | (images only — no fonts)                          | Mentioned only to clarify it's not for fonts.  |

`mutool extract` is the closest CLI equivalent — one command, dumps everything.
The big caveat: mupdf is AGPL-licensed.

## When this tool wins anyway

- You want a **GUI** with thumbnail preview — no `pip install`, no terminal.
- You're on a locked-down machine where you can run a portable JAR but not
  install Python/poppler/mupdf.
- You want a permissively-licensed (Apache 2.0) reference implementation in
  Java.
- You're building a Java app and want to copy the `processResources` method
  pattern.

## When other tools win

- Anything involving more than one PDF at a time.
- CI / automation.
- Need Type 1 / CFF support (pikepdf, PyMuPDF, mutool all handle these).
- Need to **inspect** the extracted fonts (fontTools, FontForge) — this tool
  just dumps bytes.

## Within this repo

This tool is **orthogonal** to marker / chandra / pdftext. None of those
care about the binary font files — they only use font metadata (name, size,
weight, flags) that's exposed at the text-extraction layer. If your goal is
"convert PDF to markdown", you don't need this tool. If your goal is "use
that PDF's font in InDesign," this tool (or pikepdf) is what you want.
