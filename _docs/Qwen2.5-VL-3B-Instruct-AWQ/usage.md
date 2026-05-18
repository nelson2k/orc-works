# Loading and using the model

## Install

```bash
pip install "git+https://github.com/huggingface/transformers" accelerate
pip install qwen-vl-utils[decord]==0.0.8   # video + url + base64 helpers
pip install autoawq                         # AWQ kernels
# optional but recommended:
pip install flash-attn --no-build-isolation
```

The upstream README warns about `KeyError: 'qwen2_5_vl'` if `transformers` is too old. The model class was merged in 4.49.0.dev — older versions don't recognize the `model_type`.

## Minimal inference

```python
from transformers import Qwen2_5_VLForConditionalGeneration, AutoProcessor
from qwen_vl_utils import process_vision_info

model = Qwen2_5_VLForConditionalGeneration.from_pretrained(
    "Qwen/Qwen2.5-VL-3B-Instruct-AWQ",
    torch_dtype="auto",      # AWQ scales fp16/bf16 internally
    device_map="auto",
)

processor = AutoProcessor.from_pretrained("Qwen/Qwen2.5-VL-3B-Instruct-AWQ")

messages = [{
    "role": "user",
    "content": [
        {"type": "image", "image": "file:///path/to/photo.jpg"},
        {"type": "text",  "text": "Describe this image."},
    ],
}]

text = processor.apply_chat_template(messages, tokenize=False, add_generation_prompt=True)
image_inputs, video_inputs = process_vision_info(messages)
inputs = processor(text=[text], images=image_inputs, videos=video_inputs,
                   padding=True, return_tensors="pt").to("cuda")

out_ids = model.generate(**inputs, max_new_tokens=128)
out_trim = [o[len(i):] for i, o in zip(inputs.input_ids, out_ids)]
print(processor.batch_decode(out_trim, skip_special_tokens=True)[0])
```

To load from this local snapshot instead of the Hub, replace the model id with the absolute path to `repos-folder/Qwen2.5-VL-3B-Instruct-AWQ` (Windows path is fine — HF resolves either).

## With Flash-Attention 2

```python
import torch
model = Qwen2_5_VLForConditionalGeneration.from_pretrained(
    "Qwen/Qwen2.5-VL-3B-Instruct-AWQ",
    torch_dtype=torch.bfloat16,
    attn_implementation="flash_attention_2",
    device_map="auto",
)
```

Big win for multi-image and video prompts.

## Image sources

The `qwen_vl_utils.process_vision_info` helper resolves three formats inside content blocks:

```python
{"type": "image", "image": "file:///abs/path/to/img.jpg"}     # local
{"type": "image", "image": "https://example.com/img.jpg"}     # url
{"type": "image", "image": "data:image;base64,/9j/..."}       # base64
```

For videos, only **local files** are supported (the processor needs random access). For URL/base64 video, download first.

## Controlling token cost

Image token count = `(H/28) × (W/28)`. Cap it at processor construction time:

```python
processor = AutoProcessor.from_pretrained(
    "Qwen/Qwen2.5-VL-3B-Instruct-AWQ",
    min_pixels=256  * 28 * 28,   # ≥256  image tokens
    max_pixels=1280 * 28 * 28,   # ≤1280 image tokens
)
```

Or per-message:

```python
{"type": "image", "image": "...", "resized_height": 280, "resized_width": 420}
```

See [image-pipeline.md](image-pipeline.md) for the full rules.

## Sampling

`generation_config.json` has `temperature=0.1, top_k=1, top_p=0.001` — effectively greedy. Override on each call:

```python
model.generate(**inputs, max_new_tokens=512,
               do_sample=True, temperature=0.7, top_p=0.9)
```

## Structured output

To get JSON-shaped answers, just ask for them in the prompt — the upstream evals report stable JSON output for invoices, forms, tables. For bounding boxes specifically, the model emits inline `<|box_start|>(x1,y1),(x2,y2)<|box_end|>` spans inside the assistant text; parse them out post-hoc rather than expecting a separate detection head. See [tokenizer-and-templates.md](tokenizer-and-templates.md#special-tokens).
