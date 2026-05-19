# Core Pipeline

## Purpose

The core pipeline exists for self-hosted mode. It turns document inputs into page images, detects layout regions, recognizes each region through an OCR/VLM API, and formats the result into JSON and Markdown.

The central class is `Pipeline` in `glmocr/pipeline/pipeline.py`.

## Pipeline Object Composition

`Pipeline` builds these components:

- `PageLoader`: loads images/PDF pages and builds OCR request payloads.
- `OCRClient`: sends recognition requests to the configured OCR service.
- `ResultFormatter`: post-processes OCR output and emits JSON/Markdown.
- `PPDocLayoutDetector`: runs local layout detection using PP-DocLayoutV3.

The constructor accepts optional custom `layout_detector` and `result_formatter`, so these parts are extension points.

## Stage 1: Page Loading

`PageLoader` is defined in `glmocr/dataloader/page_loader.py`.

Responsibilities:

- Load image files.
- Load PDF files as page images.
- Load `file://` URLs.
- Load image bytes.
- Load PDF bytes.
- Load `data:image/...` base64 URLs.
- Stream PDF pages one at a time.
- Build OpenAI-style OCR API request payloads.

PDF controls:

- `pdf_dpi`
- `pdf_max_pages`
- `pdf_verbose`

Image/request controls:

- `max_tokens`
- `temperature`
- `top_p`
- `top_k`
- `repetition_penalty`
- `image_format`
- `min_pixels`
- `max_pixels`
- `task_prompt_mapping`

The page loader can build different prompts for task types such as:

- `text`
- `table`
- `formula`

## Stage 2: Layout Detection

`PPDocLayoutDetector` is defined in `glmocr/layout/layout_detector.py`.

Responsibilities:

- Load `PPDocLayoutV3ImageProcessor`.
- Load `PPDocLayoutV3ForObjectDetection`.
- Choose device from explicit config, CUDA availability, or CPU.
- Run model inference in batches.
- Apply post-processing.
- Filter detections by class threshold if configured.
- Convert detected boxes and polygons into normalized 0-1000 coordinates.
- Map native labels into task types.
- Generate optional layout visualization images.

Key config:

- `model_dir`
- `threshold`
- `threshold_by_class`
- `batch_size`
- `cuda_visible_devices`
- `device`
- `use_polygon`
- `layout_nms`
- `layout_unclip_ratio`
- `layout_merge_bboxes_mode`
- `label_task_mapping`
- `id2label`

Task mapping behavior:

- `text`: send to OCR with text prompt.
- `table`: send to OCR with table prompt.
- `formula`: send to OCR with formula prompt.
- `skip`: keep region, crop image, do not OCR.
- `abandon`: discard region.

The default config abandons headers, footers, page numbers, footnotes, references, and similar page furniture. It skips image/chart regions and preserves them as image outputs.

## Stage 3: Recognition

Recognition happens in `recognition_worker` inside `glmocr/pipeline/_workers.py`.

Behavior:

- Consumes region messages from `region_queue`.
- For `skip` regions, stores the cropped image and records the region with no text content.
- For OCR regions, builds an OCR request from the cropped image.
- Sends requests concurrently through `ThreadPoolExecutor`.
- Caps concurrency at `min(max_workers, 128)`.
- Stores each completed region result by page.

Recognition output is normalized to:

```json
{
  "choices": [
    {
      "message": {
        "content": "recognized text"
      }
    }
  ]
}
```

If recognition fails, the region content is set to `None`.

## Worker Communication

The queue message identifiers are defined in `glmocr/pipeline/_common.py`.

`page_queue` messages:

- `image`: contains `page_idx`, `unit_idx`, and a PIL image.
- `unit_done`: tells layout worker all pages for one input unit have been queued.
- `done`: tells layout worker no more pages are coming.

`region_queue` messages:

- `region`: contains `page_idx`, cropped image, and region metadata.
- `done`: tells recognition worker no more regions are coming.

## State Management

`PipelineState` in `glmocr/pipeline/_state.py` stores:

- `page_queue`
- `region_queue`
- page images
- layout results
- recognition results by page
- cropped image regions
- layout visualization images
- exceptions
- shutdown event
- attached `UnitTracker`

It provides thread-safe result access and cleanup after each unit is emitted.

## Unit Tracking

`UnitTracker` in `glmocr/pipeline/_unit_tracker.py` tracks per-input completion.

Its protocol:

1. `register_page(page_idx, unit_idx)` records ownership of pages.
2. `finalize_unit(unit_idx, region_count)` records how many regions that input produced.
3. `on_region_done(page_idx)` increments completed regions.
4. `wait_next_ready_unit()` lets the main thread wait for complete units.

This design supports streaming output, multi-page PDFs, and out-of-order recognition.

## Result Formatting

`ResultFormatter` is defined in `glmocr/postprocess/result_formatter.py`.

It handles:

- OCR-only formatting.
- Layout-mode grouped region formatting.
- Label mapping into `text`, `table`, `formula`, and `image`.
- Empty content filtering.
- Formula wrapping in display math blocks.
- Title conversion to Markdown headings.
- Bullet point normalization.
- Code block closing.
- Hyphenated word merging.
- Formula-number merging with `\tag{...}`.
- Cropped image reference generation.
- JSON serialization.
- Markdown generation.

Final JSON is a list of pages, where each page is a list of region dictionaries.

Final Markdown is page content concatenated with blank lines.

## Passthrough OCR Mode

If a request contains no image sources, `Pipeline._process_passthrough()` forwards the request to the OCR API directly and formats the returned OCR content as a single text region.

## Error Handling

Errors in workers are recorded in `PipelineState`.

When a worker records an exception:

- Shutdown is requested.
- The tracker is signaled.
- The main thread unblocks.
- After joining workers, `state.raise_if_exceptions()` raises a combined runtime error.

A health watchdog also checks whether the OCR service remains reachable during processing.
