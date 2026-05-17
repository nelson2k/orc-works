# Schema (`pdftext/schema.py`)

All types are `TypedDict`s except `Bbox` (a regular class) and `Reference` /
`PageReference` (dataclass / class).

## `Bbox`

Internal-only wrapper around a 4-float list. Stripped to a plain list before
the public API returns. Useful methods:

| Method                          | Notes                                                       |
|---------------------------------|-------------------------------------------------------------|
| `.bbox`                         | The raw `[x1, y1, x2, y2]` list                             |
| `.width / .height / .area / .center / .size` | Computed                                      |
| `.x_start / .y_start / .x_end / .y_end`      | Index aliases                                 |
| `.merge(other)`                 | Returns new `Bbox` enclosing both                           |
| `.overlap_x(other) / .overlap_y(other)` | 1-D overlap distance                                |
| `.intersection_area(other)`     | Pixel area                                                  |
| `.intersection_pct(other)`      | `intersection / self.area`                                  |
| `.rotate(page_w, page_h, deg)`  | 90/180/270 rotation around the page                         |
| `.rescale(img_size, page)`      | Scale bbox into image-space coords                          |

`ensure_nonzero_area=True` in `__init__` clamps degenerate boxes by adding 1
to the right/bottom (used for zero-width chars in link merging).

## `Char` (TypedDict)

```python
{
    "bbox": Bbox,
    "char": str,               # single unicode char
    "rotation": float,          # radians, from pdfium
    "font": {
        "name": str | None,
        "flags": int,           # PDF spec 1.7 §5.7.1 Font Descriptor Flags
        "size": float,
        "weight": int,          # 100-900
    },
    "char_idx": int,            # position in textpage.count_chars()
}
```

## `Span` (TypedDict)

```python
{
    "bbox": Bbox,
    "text": str,
    "font": {…},                # same as Char
    "chars": list[Char],        # dropped from output unless keep_chars=True
    "char_start_idx": int,
    "char_end_idx": int,
    "rotation": int,
    "url": str,                 # filled by add_links_and_refs; "" otherwise
    "superscript": bool,        # set by assign_scripts
    "subscript": bool,
}
```

## `Line` / `Block` / `Page`

```python
Line  = {"spans": list[Span], "bbox": Bbox, "rotation": int}
Block = {"lines": list[Line], "bbox": Bbox, "rotation": int}
Page  = {
    "page": int, "bbox": Bbox, "width": int, "height": int,
    "blocks": list[Block], "rotation": int, "refs": list[Reference],
}
```

After `dictionary_output` returns, all `bbox` fields are converted from
`Bbox` to plain `[x1, y1, x2, y2]` lists.

## Links + references

```python
Link = {
    "page": int, "bbox": list[float],
    "dest_page": int | None, "dest_pos": [x, y] | None,
    "url": str | None,
}

@dataclass
class Reference:
    idx: int          # 0-based within its page
    page: int
    coord: list[float]
    @property
    def ref(self): return f"page-{self.page}-{self.idx}"
    @property
    def url(self): return f"#{self.ref}"

class PageReference:
    page_ref_map: dict[int, list[Reference]]
    def add_ref(page, coord) -> Reference   # de-dups by coord
    def get_refs(page) -> list[Reference]
```

`Reference.url` is what gets stuffed into `Span.url` when a link points to an
internal page; the destination page's `Page.refs` carries the anchors.

## Tables

```python
TableCell  = {"text": str, "bbox": Bbox}
TableInput = {"tables": list[list[int]], "img_size": [w, h]}
```

See [tables.md](tables.md).

## Aliases at the bottom of `schema.py`

```python
Chars = List[Char]; Spans = List[Span]; Lines = List[Line]
Blocks = List[Block]; Pages = List[Page]
Tables = List[List[TableCell]]; TableInputs = List[TableInput]
```
