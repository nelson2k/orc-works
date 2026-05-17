# Schema

Dataclasses in [chandra/model/schema.py](../../repos-folder/chandra/chandra/model/schema.py).

## `BatchInputItem`

```python
@dataclass
class BatchInputItem:
    image: Image.Image
    prompt: str | None = None        # fully custom prompt; overrides prompt_type
    prompt_type: str | None = None   # key into PROMPT_MAPPING ("ocr_layout" | "ocr")
```

If `prompt` is not provided, the backend looks up `PROMPT_MAPPING[prompt_type]` (in `chandra/prompts.py`). At least one must be set.

## `GenerationResult`

Internal — what each backend (`generate_hf` / `generate_vllm`) returns per item before output parsing.

```python
@dataclass
class GenerationResult:
    raw: str           # the model's full HTML text
    token_count: int   # completion tokens
    error: bool = False
```

## `BatchOutputItem`

What `InferenceManager.generate(...)` returns per input item.

```python
@dataclass
class BatchOutputItem:
    markdown: str       # post-processed markdown
    html: str           # post-processed HTML (top-level divs unwrapped)
    chunks: dict        # list of {bbox, label, content} (despite the type hint name)
    raw: str            # untouched model output
    page_box: list[int] # [0, 0, image.width, image.height]
    token_count: int    # from GenerationResult
    images: dict        # {filename.webp: PIL.Image} for extracted Image/Figure blocks
    error: bool         # propagated from GenerationResult
```

Note: `chunks` is annotated `dict` but is actually `list[dict]` — each entry is a serialized `LayoutBlock` with keys `bbox: list[int]`, `label: str`, `content: str`. Bboxes here are in pixel coordinates of the source image (after rescaling from the 0–1000 normalized space).

## `LayoutBlock`

In [chandra/output.py](../../repos-folder/chandra/chandra/output.py):

```python
@dataclass
class LayoutBlock:
    bbox: list[int]   # [x0, y0, x1, y1] in image pixels
    label: str        # one of the labels listed in prompts.py
    content: str      # inner HTML of the block (with nested data-bbox stripped)
```
