# PDF Editing

## Merge PDFs

```python
import pymupdf

out = pymupdf.open()
for path in ["a.pdf", "b.pdf"]:
    with pymupdf.open(path) as src:
        out.insert_pdf(src)

out.save("merged.pdf")
```

`insert_pdf` keyword args of interest: `from_page=-1, to_page=-1, start_at=-1, rotate=-1, links=1, annots=1, widgets=1`.

## Split pages

```python
doc = pymupdf.open("input.pdf")

for i in range(doc.page_count):
    out = pymupdf.open()
    out.insert_pdf(doc, from_page=i, to_page=i)
    out.save(f"page-{i + 1}.pdf")
```

## Delete and reorder pages

```python
doc.delete_page(2)              # by index
doc.delete_pages([0, 4, 7])     # list of indices
del doc[3]                      # also supported (__delitem__)
del doc[2:5]                    # slice form

doc.select([3, 0, 1, 2])        # reorder/keep this page list
```

## Save / serialize

```python
doc.save("out.pdf",
         garbage=3,             # 0-4: aggressive object cleanup
         deflate=True,
         clean=True,
         use_objstms=1,
         encryption=pymupdf.PDF_ENCRYPT_AES_256,
         owner_pw="secret")

data = doc.tobytes(garbage=3, deflate=True)   # in-memory equivalent (alias for doc.write())
```

`Document.save` supports incremental saves (`incremental=True` overwrites in place; no `garbage` allowed). `ez_save` is a convenience preset with `garbage=3, clean=True, deflate=True`.

## Add text

```python
page.insert_text(
    point=pymupdf.Point(72, 72),
    text="DRAFT",
    fontsize=24,
    color=(1, 0, 0),
)
```

For richer text placement use `page.insert_textbox(rect, text, ...)` or the `TextWriter` class.

## Insert images / fonts

```python
page.insert_image(rect, filename="logo.png", keep_proportion=True)
page.insert_image(rect, pixmap=pix, overlay=True)

doc.insert_font(fontname="HeBo", fontfile="HelveticaBold.ttf")  # then reference "HeBo" in insert_text
```

## Annotations

The `Page` class has matching `add_*_annot` methods for every standard PDF annotation kind:

`add_caret_annot`, `add_circle_annot`, `add_file_annot`, `add_freetext_annot`, `add_highlight_annot`, `add_ink_annot`, `add_line_annot`, `add_polygon_annot`, `add_polyline_annot`, `add_rect_annot`, `add_redact_annot`, `add_squiggly_annot`, `add_stamp_annot`, `add_strikeout_annot`, `add_text_annot`, `add_underline_annot`.

```python
hits = page.search_for("confidential", quads=True)
if hits:
    page.add_highlight_annot(hits)

page.add_freetext_annot(rect, "Reviewed", fontsize=11, text_color=(0, 0, 1))
```

## Redaction

Redaction is two-step: add redaction annotations, then apply them.

```python
for rect in page.search_for("secret"):
    page.add_redact_annot(rect, fill=(1, 1, 1))

page.apply_redactions()
doc.save("redacted.pdf")
```

`apply_redactions(images=2, graphics=1, text=0)` controls what overlapping objects do:
- `images`: 0 ignore, 1 remove all overlapping, 2 blank out overlapping parts, 3 remove unless invisible.
- `graphics`: 0 ignore, 1 remove if fully contained, 2 remove all overlapping.
- `text`: 0 remove, 1 ignore.

After `apply_redactions()`, the underlying content is removed from the saved PDF.

## Table of contents (outline)

```python
toc = doc.get_toc(simple=True)   # [[level, title, page], ...]
toc.append([1, "New chapter", 42])
doc.set_toc(toc)
```

## Metadata

```python
print(doc.metadata)

doc.set_metadata({
    "title": "Processed document",
    "author": "OCR pipeline",
})
```

## Convert non-PDF to PDF

```python
src = pymupdf.open("book.epub")
pdf_bytes = src.convert_to_pdf()      # bytes
pdf_doc = pymupdf.open("pdf", pdf_bytes)
```

Useful for normalizing input before running OCR or text extraction.
