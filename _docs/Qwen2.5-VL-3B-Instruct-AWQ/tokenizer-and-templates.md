# Tokenizer and chat template

From `tokenizer_config.json`, `added_tokens.json`, `chat_template.json`.

## Tokenizer basics

| Property | Value |
|---|---|
| `tokenizer_class` | `Qwen2Tokenizer` (byte-level BPE) |
| `vocab_size` (config) | 151 936 |
| `model_max_length` | 131 072 (128 K) |
| `add_bos_token` | false |
| `add_prefix_space` | false |
| `bos_token` | none in tokenizer; config uses 151 643 (`<|endoftext|>`) |
| `eos_token` | `<|im_end|>` (151 645) |
| `pad_token` | `<|endoftext|>` (151 643) |
| `clean_up_tokenization_spaces` | false |
| `errors` | `replace` |

Files on disk:

- `vocab.json` (2.6 MB) — token → id table.
- `merges.txt` (1.7 MB) — BPE merges.
- `tokenizer.json` (10.9 MB) — fast-tokenizer pre-baked rules (HF `tokenizers` crate).
- `added_tokens.json`, `special_tokens_map.json` — special-token metadata.

## Special tokens

(IDs from `added_tokens.json`.)

| Token | ID | Purpose |
|---|---|---|
| `<|endoftext|>` | 151 643 | BOS/pad/end-of-doc |
| `<|im_start|>` | 151 644 | ChatML role open |
| `<|im_end|>` | 151 645 | ChatML role close, EOS |
| `<|object_ref_start|>` | 151 646 | Begin object reference |
| `<|object_ref_end|>` | 151 647 | End object reference |
| `<|box_start|>` | 151 648 | Begin bounding-box `(x1,y1),(x2,y2)` |
| `<|box_end|>` | 151 649 | End bounding-box |
| `<|quad_start|>` | 151 650 | Begin quadrilateral (4 points) |
| `<|quad_end|>` | 151 651 | End quadrilateral |
| `<|vision_start|>` | 151 652 | Begin vision span (image or video) |
| `<|vision_end|>` | 151 653 | End vision span |
| `<|vision_pad|>` | 151 654 | Generic vision placeholder |
| `<|image_pad|>` | 151 655 | Image-patch placeholder slot |
| `<|video_pad|>` | 151 656 | Video-patch placeholder slot |
| `<tool_call>` | 151 657 | (non-special-special) tool-call open |
| `</tool_call>` | 151 658 | Tool-call close |
| `<|fim_prefix|>` | 151 659 | FIM prefix (code) |
| `<|fim_middle|>` | 151 660 | FIM middle |
| `<|fim_suffix|>` | 151 661 | FIM suffix |
| `<|fim_pad|>` | 151 662 | FIM pad |
| `<|repo_name|>` | 151 663 | Repo name marker (code) |
| `<|file_sep|>` | 151 664 | File separator (code) |

The spatial tokens (`object_ref`, `box`, `quad`) are how the model emits localization output — bounding boxes are simply text like `<|box_start|>(123,456),(789,1011)<|box_end|>` inside the answer stream. No separate region head; no special-spatial-embedding routing. Coordinates are inline integers.

The FIM and repo/file tokens are inherited from Qwen2.5-Coder ancestry and aren't typically used in vision workloads, but they're in vocab.

## Chat template

From `chat_template.json` (also baked into `tokenizer_config.json`):

```jinja
{% for message in messages %}
  {% if loop.first and message.role != 'system' %}
<|im_start|>system
You are a helpful assistant.<|im_end|>
  {% endif %}
<|im_start|>{{ message.role }}
  {% if message.content is string %}
    {{ message.content }}<|im_end|>
  {% else %}
    {% for content in message.content %}
      {% if content.type == 'image' or 'image' in content %}
        {% if add_vision_id %}Picture N: {% endif %}<|vision_start|><|image_pad|><|vision_end|>
      {% elif content.type == 'video' or 'video' in content %}
        {% if add_vision_id %}Video N: {% endif %}<|vision_start|><|video_pad|><|vision_end|>
      {% elif 'text' in content %}
        {{ content.text }}
      {% endif %}
    {% endfor %}<|im_end|>
  {% endif %}
{% endfor %}
{% if add_generation_prompt %}<|im_start|>assistant
{% endif %}
```

Behavior:

- **ChatML** style: each message wrapped in `<|im_start|>role\n…<|im_end|>`.
- Auto-injects a default system prompt (`You are a helpful assistant.`) if the caller's first message isn't a system message.
- For multimodal content, each image becomes a `<|vision_start|><|image_pad|><|vision_end|>` triplet in the text; the processor then expands each `<|image_pad|>` into N copies (one per image token) before forwarding.
- `add_vision_id=True` (optional flag to `apply_chat_template`) emits `Picture 1:` / `Video 1:` labels so the model can disambiguate when multiple visuals appear in one turn.

## Generation defaults

From `generation_config.json`:

```json
{
  "bos_token_id": 151643,
  "eos_token_id": [151645, 151643],
  "pad_token_id": 151643,
  "do_sample": true,
  "temperature": 0.1,
  "top_k": 1,
  "top_p": 0.001,
  "repetition_penalty": 1.05
}
```

Notice `top_k: 1` and `top_p: 0.001` — these together make sampling effectively deterministic (it's "sampling" only nominally). For more diverse output, override at `model.generate(...)` call time with higher `temperature`, `top_p`, or `top_k`.

Both `<|im_end|>` and `<|endoftext|>` are accepted as EOS.
