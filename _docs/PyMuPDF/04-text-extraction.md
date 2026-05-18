# Text Extraction

The basic pattern is:

```python
import pymupdf

doc = pymupdf.open("document.pdf")
for page in doc:
    print(page.get_text("text"))
```

## Useful formats

```python
page.get_text("text")      # plain text
page.get_text("words")     # word tuples with positions
page.get_text("blocks")    # block-level extraction
page.get_text("dict")      # structured blocks/lines/spans
page.get_text("rawdict")   # lower-level structured details
page.get_text("html")      # HTML-ish output
page.get_text("xml")       # XML output
page.get_text("json")      # JSON output
```

For layout-sensitive work, `dict`, `rawdict`, `words`, and `blocks` are the most useful starting points.

## Reuse TextPage

Creating a `TextPage` is the expensive part. Reuse it when extracting multiple formats from the same page:

```python
tp = page.get_textpage()

text = page.get_text("text", textpage=tp)
words = page.get_text("words", textpage=tp)
data = page.get_text("dict", textpage=tp)
```

The same pattern applies to OCR:

```python
tp = page.get_textpage_ocr(dpi=300, full=True)
text = page.get_text("text", textpage=tp)
hits = page.search_for("invoice", textpage=tp)
```

## Clip regions

Use a rectangle to extract from part of a page:

```python
clip = pymupdf.Rect(50, 100, 400, 300)
text = page.get_text("text", clip=clip)
```

Coordinates are page points, not pixels.

## Search

```python
hits = page.search_for("invoice number")
for rect in hits:
    print(rect)
```

Use `quads=True` for better highlighting of rotated or non-horizontal text.
