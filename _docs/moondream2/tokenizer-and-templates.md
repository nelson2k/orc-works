# Tokenizer and prompt templates

The model uses a single byte-level BPE tokenizer with **51 200** vocabulary entries, sourced from the HF repo `moondream/starmie-v1`. The runtime fetches it at startup:

```python
self.tokenizer = Tokenizer.from_pretrained("moondream/starmie-v1")
```

(That's the `tokenizers` library's `Tokenizer`, not transformers'. The `tokenizer.json` / `vocab.json` / `merges.txt` files in this repo are kept for HF compatibility but the live tokenizer comes from the Hub.)

The June-2025 release introduced a "superword" tokenizer that emits fewer tokens for the same English text, giving 20–40 % faster generation. There is also a "lightweight tokenizer transfer hypernetwork" used during training to swap tokenizer families without retraining the LM head; that's a training-side trick — the inference runtime sees a single fixed tokenizer.

## Special token ids

(From `config.py` → `TokenizerConfig`.)

| Name | Id | Meaning |
|---|---|---|
| `bos_id` | 0 | Beginning of sequence / also doubles as EOS in some skills |
| `eos_id` | 0 | End of sequence (yes — same id as BOS) |
| `answer_id` | 3 | Separator token used inside grounded reasoning; signals "stop reasoning, start answering" |
| `thinking_id` | 4 | Inserted at the end of the prompt to trigger reasoning mode |
| `coord_id` | 5 | Placeholder for a coordinate slot |
| `size_id` | 6 | Placeholder for a size slot |
| `start_ground_points_id` | 7 | Begin a grounded-points block inside reasoning text |
| `end_ground_id` | 9 | End a grounded-points block |

The fact that `bos_id == eos_id == 0` is intentional: the model knows by **template position** whether `[0]` means "start" or "stop". The decoder explicitly suppresses `eos_id` in the logits during reasoning generation so the model can't terminate early.

## Per-skill prompt templates

(From `TokenizerConfig.templates`.)

```python
{
    "caption": {
        "short":  [1, 32708, 2, 12492, 3],
        "normal": [1, 32708, 2,  6382, 3],
        "long":   [1, 32708, 2,  4059, 3],
    },
    "query":  {"prefix": [1, 15381, 2], "suffix": [3]},
    "detect": {"prefix": [1,  7235, 476, 2], "suffix": [3]},
    "point":  {"prefix": [1,  2581, 2], "suffix": [3]},
}
```

Token `1` and `2` are template-frame markers (think `<start_of_skill>`, `<role_split>`); `3` closes the prompt. The middle ids encode the specific skill / length variant. For `detect` and `point`, the user's free-form `object` string is BPE-tokenized in between the prefix and suffix:

```
[1, 7235, 476, 2] + tokenize(" " + object) + [3]
```

For `query`:

```
[1, 15381, 2] + tokenize(question) + [3]
```

Plus an extra `[thinking_id]` (4) appended when `reasoning=True`. After the reasoning phase completes (model emits `answer_id` = 3), the answer phase re-prefills with just `[3]` (the suffix) to switch the model into "answer mode".

For `caption` there is no user input — the template **is** the entire prompt, and the model generates from there.

## Spatial placeholders

When a `query` call carries `spatial_refs`, each reference is expanded into placeholder tokens before encoding:

```python
for ref in spatial_refs:
    if len(ref) == 2:            # (x, y) point
        spatial_toks += [coord_id, coord_id]
    else:                         # (x_min, y_min, x_max, y_max) box → (center, size)
        spatial_toks += [coord_id, coord_id, size_id]
```

Those tokens get spliced in **before** the question tokens:

```
[1, 15381, 2] + spatial_toks + tokenize(question) + [3]
```

At embedding time the model replaces every position whose token == `coord_id` with `encode_coordinate(x_or_y)` and every position whose token == `size_id` with `encode_size((w, h))`. This is the same encode_* machinery that powers detection / pointing output; in the input it lets you **point the question at a place** rather than describe it in words.

## Text-only mode

If `query` is called without an image, the prompt becomes:

```
[bos_id] + [1, 15381, 2] + tokenize(question) + [3]
```

(i.e. the BOS is prepended explicitly because there is no image-prefill to start the sequence.) The attention mask switches to a plain lower-triangular causal mask of size `max_context × max_context` for this path.

## Token-streaming behavior

The text generator yields *boundary-safe* string chunks rather than per-token output:

- After a `\n`, flush whatever's accumulated.
- After a CJK character (Unicode CJK Unified Ideographs ranges), flush.
- Otherwise, only flush up to the last space — so partial words and partial UTF-8 sequences are kept buffered until they complete.
- At end of generation, flush any remaining text.

That's why `caption(stream=True)["caption"]` and `query(stream=True)["answer"]` give clean word-by-word output instead of broken byte fragments.
