# Schema

Module: [pdftext/schema.py](../../repos-folder/pdftext/pdftext/schema.py)

Defines the in-memory + JSON output shape. Most types are `TypedDict`s, so they're just plain dicts at runtime with documented keys. `Bbox` is a real class with geometry helpers.

## `Bbox`

```python
class Bbox:
    def __init__(self, bbox: List[float], ensure_nonzero_area=False)
```

Wraps `[x_min, y_min, x_max, y_max]`. With `ensure_nonzero_area=True` the constructor expands zero-width/height bboxes by 1 unit to avoid downstream divide-by-zero.

Properties:

- `height`, `width`, `area`, `size`, `center`
- `x_start`, `y_start`, `x_end`, `y_end`
- `bbox` — the raw list

Geometry methods:

- `merge(other: Bbox) -> Bbox` — minimum enclosing bbox
- `overlap_x(other)`, `overlap_y(other)` — 1-axis overlap in absolute units
- `intersection_area(other)`, `intersection_pct(other)` — intersection-over-self ratio
- `rotate(page_width, page_height, rotation)` — apply a page rotation (0 / 90 / 180 / 270) to the bbox coordinates
- `rescale(img_size: [w, h], page: Page) -> Bbox` — scale into image-pixel space

Indexable: `bbox[0]` returns `x_min`, etc. The serialization step in `extraction.dictionary_output` replaces every `Bbox` with `bbox.bbox` (the list) before returning.

## TypedDicts

```python
class Char(TypedDict):
    bbox: Bbox            # serialized as a 4-list
    char: str             # always a single grapheme (post deduplication)
    rotation: float
    font: Dict[str, Union[Any, str]]
    char_idx: int         # 0-based index in the page char stream
```

```python
class Span(TypedDict):
    bbox: Bbox
    text: str             # concatenated chars after postprocess_text
    font: Dict[str, Union[Any, str]]
    chars: List[Char]     # dropped unless keep_chars=True
    char_start_idx: int
    char_end_idx: int
    rotation: int
    url: str              # empty unless a link annotation overlaps
    superscript: bool
    subscript: bool
```

```python
class Line(TypedDict):
    spans: List[Span]
    bbox: Bbox
    rotation: int
```

```python
class Block(TypedDict):
    lines: List[Line]
    bbox: Bbox
    rotation: int
```

```python
class Page(TypedDict):
    page: int             # page index
    bbox: Bbox            # the page bbox (rotation-adjusted)
    width: int
    height: int
    blocks: List[Block]
    rotation: int
    refs: List[Reference]
```

Aliases for clarity (lowercase plural type names in code):

- `Pages = List[Page]`
- `Blocks = List[Block]`
- `Lines = List[Line]`
- `Spans = List[Span]`
- `Chars = List[Char]`

## Tables

```python
class TableCell(TypedDict):
    text: str
    bbox: Bbox

class TableInput(TypedDict):
    tables: List[List[int]]    # cell bboxes in image-space
    img_size: List[int]        # [w, h] for re-scaling

TableInputs = List[TableInput]
Tables      = List[TableCell]
```

`table_output(...)` returns `List[Tables]` — one `Tables` (list of `TableCell`) per input page.

## Links and references

```python
class Link(TypedDict):
    page: int
    bbox: List[float]
    dest_page: Optional[int]    # for internal links
    dest_pos: Optional[List[float]]
    url: Optional[str]          # for external links

@dataclass
class Reference:
    idx: int
    page: int
    coord: List[float]

    @property
    def ref(self) -> str: ...

@dataclass
class PageReference:
    page: int
    refs: List[Reference]
```

After `pdf.links.add_links_and_refs(pages, pdf)`:

- Each `Span.url` is set if a link bbox overlaps the span.
- Each `Page.refs` lists `Reference` entries — these are used for *internal* destinations (e.g. table of contents) so consumers can resolve in-text anchors back to page coordinates.

## `font` dict shape

The `font` dict inside `Char` and `Span` mirrors pypdfium2's font info:

| Key | Type | Notes |
|---|---|---|
| `name` | str | The PDF font name (may be subset-tagged: `ABCDEF+TimesNewRoman`) |
| `size` | float | Font size in PDF units (one unit = 1/72 inch) |
| `weight` | int | 400 = Regular, 700 = Bold, etc. |
| `flags` | int | PDF font descriptor flags (bit 1 = fixed-pitch, bit 2 = serif, bit 3 = symbolic, bit 7 = italic, ...) |

Some characters may have an empty / partial font dict if pdfium can't resolve the font; downstream consumers should treat the keys as optional.

## Why TypedDict instead of dataclasses

TypedDicts are JSON-serializable as-is (after replacing `Bbox` objects with their list form). This lets `dictionary_output` emit standards-compliant JSON via `json.dumps` without custom encoders. The trade-off is no runtime field validation, no class-level methods — the consumer is expected to follow the documented shape.
