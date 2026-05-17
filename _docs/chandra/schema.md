# Schema (`chandra/model/schema.py`)

Four small `@dataclass`es. No pydantic, no inheritance.

## `BatchInputItem`

```python
@dataclass
class BatchInputItem:
    image: Image.Image
    prompt: str | None = None       # explicit override
    prompt_type: str | None = None  # key into PROMPT_MAPPING ('ocr_layout' | 'ocr')
```

One per page. The CLI always uses `prompt_type="ocr_layout"` — you have to
build items yourself to use the bare `"ocr"` prompt or a custom string.

## `GenerationResult`

```python
@dataclass
class GenerationResult:
    raw: str           # the model's exact output (HTML)
    token_count: int
    error: bool = False
```

Backend-internal. `error=True` for vLLM transport failures; the post-processor
will still produce a `BatchOutputItem` from an empty `raw`.

## `BatchOutputItem`

```python
@dataclass
class BatchOutputItem:
    markdown: str               # rendered markdown
    html: str                   # cleaned HTML (Blank-Page dropped, etc.)
    chunks: dict                # list[LayoutBlock-as-dict] with bbox / label / content
    raw: str                    # the model's untouched HTML
    page_box: list[int]         # [0, 0, image.width, image.height]
    token_count: int
    images: dict[str, Image]    # filename → cropped PIL image (Image/Figure blocks)
    error: bool
```

Returned in input order from `InferenceManager.generate(batch)`.

## `LayoutBlock` (`chandra/output.py`)

```python
@dataclass
class LayoutBlock:
    bbox: list[int]  # [x0, y0, x1, y1] in actual pixel coords
    label: str       # see prompts.md for the full set
    content: str     # inner HTML of the block <div>
```

Produced by `parse_layout(raw_html, image, bbox_scale=1000)`. `chunks` in the
output is `[asdict(b) for b in layout]`.

The model emits bboxes normalized to **0..1000** (`BBOX_SCALE` in
`settings.py`). `parse_layout` scales them to the actual page image
dimensions and clamps to bounds.
