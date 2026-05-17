# Output schema

Every result type is a pydantic model. They all build on `PolygonBox` from [common/polygon.py](../../repos-folder/surya/surya/common/polygon.py).

## `PolygonBox`

```python
class PolygonBox(BaseModel):
    polygon: List[List[float]]      # 4 [x, y] points in clockwise order from top-left
    confidence: Optional[float] = None
```

Constructible from either:

- a 4-corner polygon `[[x1,y1], [x2,y2], [x3,y3], [x4,y4]]`,
- or a `[x_min, y_min, x_max, y_max]` axis-aligned bbox, which gets expanded into a polygon by the validator,
- or a `(4, 2)` numpy array.

Computed `bbox` property always returns `[min_x, min_y, max_x, max_y]`. Methods on the class include `rescale(from_size, to_size)`, `round(divisor)`, `fit_to_bounds(bbox)`, `merge(others)`, `intersection_pct(other)`, `minimum_gap(other)`, `expand(x_frac, y_frac)`.

## Detection

[detection/schema.py](../../repos-folder/surya/surya/detection/schema.py):

```python
class TextDetectionResult(BaseModel):
    bboxes: List[PolygonBox]        # detected text lines
    heatmap: Optional[Any]           # None unless include_maps=True
    affinity_map: Optional[Any]      # None unless include_maps=True
    image_bbox: List[float]          # [0, 0, width, height] of the rendered page
```

## Recognition

[recognition/schema.py](../../repos-folder/surya/surya/recognition/schema.py):

```python
class BaseChar(PolygonBox):
    text: str
    confidence: Optional[float] = 0

class TextChar(BaseChar):
    bbox_valid: bool = True          # false for special tokens / math glyphs

class TextWord(BaseChar):
    bbox_valid: bool = True

class TextLine(BaseChar):
    chars: List[TextChar]
    original_text_good: bool = False  # true when input_text already matched the image
    words: List[TextWord] | None = None

class OCRResult(BaseModel):
    text_lines: List[TextLine]
    image_bbox: List[float]
```

`original_text_good` is set when the caller supplied `input_text` (e.g. native PDF text) and the model decided it didn't need to be re-recognized — useful for layered OCR strategies.

## Layout

[layout/schema.py](../../repos-folder/surya/surya/layout/schema.py):

```python
class LayoutBox(PolygonBox):
    label: str                       # "Text" | "SectionHeader" | "Picture" | ...
    position: int                    # reading-order index, 0-based per page
    top_k: Optional[Dict[str, float]] = None   # alternative labels with confidences

class LayoutResult(BaseModel):
    bboxes: List[LayoutBox]
    image_bbox: List[float]
    sliced: bool = False             # True when the page was processed in slices
```

Labels (from [layout/label.py: LAYOUT_PRED_RELABEL](../../repos-folder/surya/surya/layout/label.py)):

```
PageHeader, PageFooter, Footnote, Picture, Figure, Text, Caption,
ListItem, SectionHeader, Table, TableOfContents, Form, Equation, Code
```

(The model also emits `<complex-block>`, which is mapped to `Figure`.)

## Table recognition

[table_rec/schema.py](../../repos-folder/surya/surya/table_rec/schema.py):

```python
class TableCell(PolygonBox):
    row_id: int
    colspan: int
    within_row_id: int               # column index within the row
    cell_id: int
    is_header: bool
    rowspan: int | None = None
    merge_up: bool = False
    merge_down: bool = False
    col_id: int | None = None
    text_lines: List[dict] | None = None

class TableRow(PolygonBox):
    row_id: int
    is_header: bool

class TableCol(PolygonBox):
    col_id: int
    is_header: bool

class TableResult(BaseModel):
    cells: List[TableCell]
    unmerged_cells: List[TableCell]  # before merge_up/merge_down resolution
    rows: List[TableRow]
    cols: List[TableCol]
    image_bbox: List[float]
```

`merge_up` / `merge_down` flags drive multi-row cells; `unmerged_cells` lets you see the model's raw output before the merge pass.

## OCR-error detection

[ocr_error/schema.py](../../repos-folder/surya/surya/ocr_error/schema.py):

```python
class OCRErrorDetectionResult(BaseModel):
    texts: List[str]                 # input texts, echoed back
    labels: List[str]                # one of "good" / "bad" per text
```

## Foundation model internals

[foundation/__init__.py](../../repos-folder/surya/surya/foundation/__init__.py) defines a few internal-only dataclasses you may encounter when reading the code:

- `ContinuousBatchInput` / `ContinuousBatchOutput` — packed tensors for the autoregressive batching loop.
- `FoundationPrompt(id, task_name, image, text, math_mode)` — a single queued item in the prompt queue.
- `TaskNames` — class with the five constants `block_without_boxes`, `ocr_with_boxes`, `ocr_without_boxes`, `layout`, `table_structure`.
