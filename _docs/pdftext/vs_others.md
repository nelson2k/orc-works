# pdftext vs PyMuPDF / pdfplumber / pypdfium2

All four extract text from PDFs. They differ in licensing, output detail,
and accuracy.

| Tool        | License  | Backend     | Output detail               | Speed      | Notes                                     |
|-------------|----------|-------------|-----------------------------|-----------:|-------------------------------------------|
| **PyMuPDF** | AGPL-3.0 | MuPDF       | blocks/lines/spans/chars    | Fast       | Reference quality but copyleft.           |
| **pdftext** | Apache-2 | pypdfium2   | blocks/lines/spans/chars    | ~4× slower than PyMuPDF | The AGPL-free replacement.                |
| **pdfplumber** | MIT   | pdfminer.six| words + bboxes              | ~10× slower than PyMuPDF | Great visual debugger; no block grouping. |
| **pypdfium2** | Apache-2| PDFium      | raw chars                    | Fastest    | No block/line grouping at all.            |

pdftext is essentially "pypdfium2 + the grouping passes you'd have to write
yourself anyway, packaged with deduplication and reading-order sort."

## Why marker uses pdftext (not PyMuPDF)

Marker's [PdfProvider](../marker/providers.md) wraps pdftext to extract the
text layer of digital PDFs. Reasons:

- **License.** marker is GPL-3.0, which is compatible with Apache-2 deps
  but **not** with AGPL deps. PyMuPDF would force marker downstream users
  into AGPL.
- **Same backend.** pypdfium2 powers both pdftext (text) and marker's image
  rendering — one PDF library to install, one set of bugs to know.
- **Structured output.** marker needs blocks + lines + per-char info to
  reconcile against surya's detection; pdftext gives that out of the box.
- **Per-page link → ref mapping.** Used by marker's `ReferenceProcessor` to
  build internal cross-references in the rendered markdown.

When marker decides a page needs OCR (low text confidence per the
ocr_error_model), it discards pdftext's output for that page and re-runs
surya recognition instead. Otherwise pdftext's spans are kept verbatim and
just re-flowed into marker's block tree.

## Why you'd use pdftext directly

- You only want text + bboxes + fonts, no layout/OCR.
- You want a fast, scriptable CLI.
- You're building a tool that needs `--workers N` multi-process page
  extraction without writing the harness yourself.
- You need link annotation → span URL stitching (most PDF libraries make
  you do this manually).

## Why you wouldn't

- Scanned/image PDFs (no OCR — try chandra / marker).
- Complex magazine-style layouts (pdftext's reading-order sort is a
  y-bucket heuristic, not a column model).
- You need rendered page images (use pypdfium2 directly).
- You need form-field semantics (pdftext can `--flatten_pdf` to merge them
  into text, but doesn't expose the field structure).
