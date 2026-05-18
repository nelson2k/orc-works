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

## Split pages

```python
doc = pymupdf.open("input.pdf")

for i in range(doc.page_count):
    out = pymupdf.open()
    out.insert_pdf(doc, from_page=i, to_page=i)
    out.save(f"page-{i + 1}.pdf")
```

## Add text

```python
page.insert_text(
    point=pymupdf.Point(72, 72),
    text="DRAFT",
    fontsize=24,
    color=(1, 0, 0),
)
```

## Highlight search results

```python
hits = page.search_for("confidential", quads=True)
if hits:
    page.add_highlight_annot(hits)
```

## Redaction

Redaction is two-step: add redaction annotations, then apply them.

```python
for rect in page.search_for("secret"):
    page.add_redact_annot(rect, fill=(1, 1, 1))

page.apply_redactions()
doc.save("redacted.pdf")
```

After `apply_redactions()`, the underlying content is removed from the saved PDF.

## Metadata

```python
print(doc.metadata)

doc.set_metadata({
    "title": "Processed document",
    "author": "OCR pipeline",
})
```
