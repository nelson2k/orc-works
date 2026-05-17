# CLI

Seven entry points, all built on `click` and sharing the [`CLILoader.common_options`](../../repos-folder/surya/surya/scripts/config.py) flags.

## Shared flags

Every conversion CLI (`surya_detect`, `surya_ocr`, `surya_layout`, `surya_table`, `surya_latex_ocr`) accepts:

| Flag | Default | Notes |
|---|---|---|
| `INPUT_PATH` | required | A file (image or PDF) or a folder |
| `--output_dir PATH` | `results/surya/<input-stem>/` | Where `results.json` and (optionally) debug images go |
| `--page_range TEXT` | full doc | e.g. `0,5-10,20` — zero-indexed, dedup-sorted |
| `--images` | off | Save annotated overlays of detected regions |
| `--debug / -d` | off | Extra logging + heatmap dumps |

Output always ends up in `OUTPUT_DIR/<input-stem>/results.json`. Folder inputs use the folder name as the stem; file inputs use the basename without extension.

## `surya_ocr`

[scripts/ocr_text.py](../../repos-folder/surya/surya/scripts/ocr_text.py). Detect lines, then OCR each line. Extra flags:

- `--task_name {ocr_with_boxes, ocr_without_boxes, block_without_boxes}` — default `ocr_with_boxes`. `ocr_without_boxes` trades line bboxes for slightly better text quality. `block_without_boxes` is for whole-block (paragraph, equation) prompts.
- `--disable_math` — by default surya recognizes inline math; this flag turns that off.

Output per page: `text_lines` (with `text`, `confidence`, `polygon`, `bbox`, `chars`, `words`), `page`, `image_bbox`. Each character has `text`, `bbox`, `polygon`, `confidence`, `bbox_valid` (false for special tokens / math). Words are computed from characters.

## `surya_detect`

[scripts/detect_text.py](../../repos-folder/surya/surya/scripts/detect_text.py). Line-level text detection only — no recognition. Output: `bboxes` (each with `bbox`, `polygon`, `confidence`) plus `vertical_lines` and `image_bbox`.

## `surya_layout`

[scripts/detect_layout.py](../../repos-folder/surya/surya/scripts/detect_layout.py). Layout-block detection + reading order in one shot. Output: `bboxes` with `bbox`, `polygon`, `position` (reading order index, 0-based), `label` (one of `Caption`, `Footnote`, `Formula`, `List-item`, `Page-footer`, `Page-header`, `Picture`, `Figure`, `Section-header`, `Table`, `Form`, `Table-of-contents`, `Handwriting`, `Text`, `Text-inline-math`), and `top_k` (a dict mapping label → confidence).

## `surya_table`

[scripts/table_recognition.py](../../repos-folder/surya/surya/scripts/table_recognition.py). Detects tables (unless `--skip_table_detection` is set), then recognizes row/column/cell structure. Extra flags:

- `--detect_boxes` — re-detect cell boxes instead of pulling them from the PDF text layer.
- `--skip_table_detection` — assume the input image is already cropped to a single table.

Output per page is a list of tables. Each table has `rows` (id + bbox + is_header), `cols` (same), `cells` (bbox + text + row_id + col_id + colspan + rowspan + is_header), `table_idx`, plus `page` and `image_bbox`.

## `surya_latex_ocr`

[scripts/ocr_latex.py](../../repos-folder/surya/surya/scripts/ocr_latex.py). Wraps the texify VLM. Input images must already be cropped to a single equation. Output format mirrors `surya_ocr`.

## `surya_gui` / `texify_gui`

[scripts/run_streamlit_app.py](../../repos-folder/surya/surya/scripts/run_streamlit_app.py) and [scripts/run_texify_app.py](../../repos-folder/surya/surya/scripts/run_texify_app.py). Launchers that shell out to streamlit with the right entry script and no file-watcher.

## Finetuning

[scripts/finetune_ocr.py](../../repos-folder/surya/surya/scripts/finetune_ocr.py). Built on HuggingFace `Trainer`, supports `torchrun` and DeepSpeed. Minimal example:

```
python surya/scripts/finetune_ocr.py \
  --output_dir ./out \
  --dataset_name datalab-to/ocr_finetune_example \
  --per_device_train_batch_size 64 \
  --gradient_checkpointing true \
  --max_sequence_length 1024
```

`--pretrained_checkpoint_path` is optional; without it the default surya OCR weights are loaded as the initialization.

## Misc utilities

[scripts/hf_to_s3.py](../../repos-folder/surya/surya/scripts/hf_to_s3.py) is a maintainer helper to mirror HF checkpoints to the datalab S3 bucket. Not part of the public surface.
