# Skills (the public API)

The model exposes four image-grounded skills plus an experimental gaze detector. They live as methods on `MoondreamModel` and are surfaced as properties on the HF wrapper (`HfMoondream`). All methods accept either a `PIL.Image` or an `EncodedImage` (the cached output of a prior `encode_image` call — useful when running multiple skills against the same image).

## caption(image, length="normal", stream=False, settings=None)

Generate a caption.

```python
print(model.caption(img, length="short")["caption"])
# stream as it generates
for chunk in model.caption(img, length="normal", stream=True)["caption"]:
    print(chunk, end="", flush=True)
```

- `length` — one of `"short"`, `"normal"`, `"long"`. Each maps to a different token template; the model is trained to produce roughly different caption lengths per template.
- `settings.max_tokens` — default 768.
- `settings.temperature`, `settings.top_p` — defaults 0.5, 0.3. Set `temperature=0` for greedy.
- `settings.variant` — LoRA variant id to download + apply (see lora-variants.md).
- Returns `{"caption": str}` (or `{"caption": generator}` if `stream=True`).

## query(image, question, reasoning=False, spatial_refs=None, stream=False, settings=None)

Open-ended visual question answering.

```python
print(model.query(img, "How many people are in this image?")["answer"])
print(model.query(img, "What color is the car?", reasoning=True)["answer"])

# Point the question at specific locations
points = [(0.42, 0.18)]                    # single (x, y) reference
boxes  = [(0.10, 0.10, 0.50, 0.40)]        # single (x_min, y_min, x_max, y_max) reference
print(model.query(img, "What is this?", spatial_refs=points)["answer"])
```

- `question` — string. Required.
- `image` — optional. If omitted, the model runs in text-only mode (BOS + prompt). The internal attention mask switches to a plain causal mask for this path.
- `reasoning=True` — first run `_generate_reasoning` until the `<answer>` token (`answer_id = 3`) is produced, capturing the trace and any grounding points emitted in it; then run `_generate_answer`. The return value gains a `"reasoning": {"text": str, "grounding": [...]}` field.
- `spatial_refs` — list of `(x, y)` tuples or `(x_min, y_min, x_max, y_max)` tuples in normalized 0–1 coordinates. Each ref expands to one or more `coord_id` / `size_id` placeholder tokens that get replaced in the embedding stream with the Fourier-encoded coordinates / sizes.
- Returns `{"answer": str}` plus optional `"reasoning"` field; or a streaming generator on `answer` if `stream=True`.

### Grounding output (reasoning mode)

In reasoning mode the model can emit `<start_ground_points>...<end_ground>` blocks inside its reasoning text. Each block carries a sequence of paired `<coord>` tokens that the region head decodes into normalized `(x, y)` points. The returned `grounding` is a list of:

```python
{
    "start_idx": int,   # offset into reasoning_text
    "end_idx":   int,
    "points":    [(x, y), ...]
}
```

That lets a downstream renderer overlay the model's intermediate "look here" decisions on the image.

## detect(image, object, settings=None)

Open-vocabulary object detection.

```python
faces = model.detect(img, "face")["objects"]
for box in faces:
    print(box["x_min"], box["y_min"], box["x_max"], box["y_max"])
```

- `object` — free-text class name. The model handles fine-grained queries ("blue bottle" vs. "bottle") and document-layout classes ("figure", "formula", "text").
- `settings.max_objects` — cap on detections, default 50.
- `settings.variant` — LoRA variant.
- Returns `{"objects": [{x_min, y_min, x_max, y_max}, ...]}` in normalized 0–1 coordinates.

Internally the generation loop alternates **coord → coord → size → next-token** per detection. Both x and y are pulled via `decode_coordinate` from the decoder hidden state and re-encoded via `encode_coordinate` before the next decoder step. Size logits give two 1024-bin log-scale distributions, decoded to width and height via `size = 2^((bin/1023) * 10 - 10)`. The next sampled token determines whether to continue with another detection or emit EOS.

## point(image, object, settings=None)

Open-vocabulary pointing.

```python
people = model.point(img, "person")["points"]
for p in people:
    print(p["x"], p["y"])
```

Same machinery as `detect`, but with `include_size=False` so only the `(x, y)` center is decoded per object. Returns `{"points": [{x, y}, ...]}` in normalized 0–1 coordinates.

## detect_gaze(image, eye=None, face=None, unstable_settings={})

Estimate the direction a person is looking. Two modes:

- **Fast** (default, `prioritize_accuracy=False`): pass `eye=(x, y)` — a single point inside an eye. One inference per call.
- **Accurate** (`prioritize_accuracy=True` in `unstable_settings`): pass `face={"x_min", "y_min", "x_max", "y_max"}`. The model runs `N=10` random eye-point queries inside the face box on the original image *and* `N=10` on the horizontally-flipped image, drops outlier predictions (`utils.remove_outlier_points`), and returns the mean.

Returns `{"gaze": {"x": ..., "y": ...}}` (or `{"gaze": None}` if no gaze is detected).

Marked unstable — the upstream README of recent revisions doesn't surface this as a primary skill.

## Streaming details

Streaming generators yield string chunks at word boundaries (or at newlines, or at CJK character boundaries). The text-streaming helper in `_generate_answer` keeps a small token cache and only releases bytes that won't change once future tokens are decoded — important because the underlying tokenizer is byte-level BPE and a single token can be a fragment of a multi-byte UTF-8 character.

## Sampling defaults

```python
DEFAULT_MAX_TOKENS  = 768
DEFAULT_TEMPERATURE = 0.5
DEFAULT_TOP_P       = 0.3
DEFAULT_MAX_OBJECTS = 50
```

For `detect` and `point` the prefill step uses `temperature=0, top_p=0` (greedy), then the coordinate / size decoding is purely argmax — these tasks don't sample.

## EOS rules per skill

- `caption`, `query` — EOS id = `tokenizer.eos_id = 0`.
- `query(reasoning=True)` reasoning phase — pseudo-EOS = `tokenizer.answer_id = 3`. When the reasoning loop sees this token it stops and returns control to the answer-generation phase.
- `detect`, `point` — EOS id = `tokenizer.eos_id`; also stops at `max_objects`.

During each loop, certain logits are explicitly suppressed (set to `-inf`) to prevent the model from emitting tokens that wouldn't fit the current generation context (e.g. `eos_id` and `size_id` are forbidden during the reasoning phase; `answer_id` is forbidden during the answer phase).
