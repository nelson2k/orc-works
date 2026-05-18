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
page.get_text("xhtml")     # XHTML output
page.get_text("xml")       # XML output
page.get_text("json")      # JSON output
page.get_text("rawjson")   # raw JSON output (mirrors rawdict)
```

For layout-sensitive work, `dict`, `rawdict`, `words`, and `blocks` are the most useful starting points.

## Full signature

```python
page.get_text(
    option="text",
    *,
    clip=None,         # Rect: restrict to this area
    flags=None,        # TEXTFLAGS_* bit switches (e.g. exclude images)
    textpage=None,     # reuse pre-built TextPage; ignores flags/clip
    sort=False,        # reading-order sort (best with "blocks"/"words")
    delimiters=None,   # custom word delimiters for "words" mode
    tolerance=3,
)
```

Defined in `src/utils.py:459`.

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

Full signature: `page.search_for(text, *, clip=None, quads=False, flags=None, textpage=None)`. Use `quads=True` for better highlighting of rotated or non-horizontal text.

## Concurrent multi-document text extraction

Module-level `pymupdf.get_text(path, ...)` runs `Page.get_text` across many pages, optionally in parallel:

```python
results = pymupdf.get_text(
    "big.pdf",
    pages=None,                # None = all pages
    method="mp",               # "single" | "mp" | "fork" (fork not on Windows)
    concurrency=None,          # None = number of CPUs
    option="text",
)
```

Each worker opens its own document handle (PyMuPDF is not thread-safe; multiprocessing is the supported parallelism). Defined in `src/__init__.py:24798`.
