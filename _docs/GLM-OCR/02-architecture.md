# Architecture

## Runtime Modes

The SDK has two primary runtime modes.

## MaaS Mode

MaaS mode is enabled by default in `glmocr/config.yaml`.

In this mode:

- The SDK acts as a thin wrapper around the hosted GLM-OCR layout parsing API.
- Local layout detection is not used.
- Local region-level OCR orchestration is not used.
- The SDK prepares the file, sends it to the API, receives Markdown/layout details, normalizes coordinates, and wraps the response in `PipelineResult`.

Relevant files:

- `glmocr/api.py`
- `glmocr/maas_client.py`
- `glmocr/config.py`
- `glmocr/config.yaml`

The MaaS client accepts local paths, URLs, data URIs, raw bytes, and base64-like strings. It wraps local files and bytes into `data:<mime>;base64,...` payloads. PDFs are sent as PDF data URIs. Non-JPEG/PNG image files may be re-encoded to PNG or JPEG before sending.

## Self-Hosted Mode

Self-hosted mode is selected when `pipeline.maas.enabled=false` or when the user passes `mode="selfhosted"`.

In this mode:

- The SDK loads pages locally.
- It runs layout detection locally with PP-DocLayoutV3.
- It crops detected regions.
- It sends each OCR region to an external VLM service.
- It merges region outputs into JSON and Markdown.

Relevant files:

- `glmocr/pipeline/pipeline.py`
- `glmocr/pipeline/_workers.py`
- `glmocr/dataloader/page_loader.py`
- `glmocr/layout/layout_detector.py`
- `glmocr/ocr_client.py`
- `glmocr/postprocess/result_formatter.py`

The OCR model itself is not run inside the SDK code. The SDK expects an API service at the configured `ocr_api` endpoint. That service may be vLLM, SGLang, Ollama, MLX, or another compatible endpoint.

## High-Level Data Flow

CLI/Python API input:

1. User provides an image, PDF, URL, bytes, or list of inputs.
2. `GlmOcr` loads config and selects MaaS or self-hosted mode.
3. MaaS mode forwards the document to hosted API.
4. Self-hosted mode builds an OpenAI-style request and passes it into the pipeline.
5. Output is returned as `PipelineResult`.
6. Results can be read in memory or saved to disk.

Backend app input:

1. Browser uploads file to FastAPI.
2. Backend stores the file under a task output directory.
3. A task record is created.
4. Worker acquires the task with a lock.
5. Pipeline flow converts PDF to images, calls OCR service, merges results.
6. Browser polls task status and receives Markdown/layout data.

## Self-Hosted Pipeline Stages

The self-hosted SDK pipeline has three worker stages:

1. `data_loading_worker`: loads pages from image/PDF inputs.
2. `layout_worker`: detects layout regions on pages.
3. `recognition_worker`: sends cropped regions to OCR service in parallel.

The main thread emits results when one input unit is complete.

One input unit means:

- One image file, or
- One PDF file containing one or more pages, or
- One bytes object.

PDF pages from the same PDF share one unit id, so the caller receives one `PipelineResult` for that PDF.

## Queues And Back Pressure

The pipeline uses two bounded queues:

- `page_queue`: Stage 1 to Stage 2.
- `region_queue`: Stage 2 to Stage 3.

Queue sizes come from config:

- `pipeline.page_maxsize`
- `pipeline.region_maxsize`

This keeps page rendering, layout detection, and recognition from growing memory without bound.

## Completion Tracking

`UnitTracker` tracks dynamic completion:

- Stage 1 registers page-to-unit mappings.
- Stage 2 finalizes each unit once layout is complete and the region count is known.
- Stage 3 notifies when each region is recognized.
- The main thread emits a result once all regions for a unit are done.

This allows streaming output per input unit even when later files are still being processed.

## Result Types

The main SDK result class is `PipelineResult`.

It contains:

- `json_result`
- `markdown_result`
- `original_images`
- Optional `image_files`
- Optional `raw_json_result`
- Optional `layout_vis_images`

In MaaS mode, extra metadata may be attached:

- `_maas_response`
- `_layout_visualization`
- `_data_info`
- `_usage`
- `_error`

## Coordinate System

The SDK normalizes layout coordinates to a 0-1000 coordinate system.

In MaaS mode, API bboxes may arrive in absolute pixel coordinates, so `GlmOcr` normalizes them using page width and height from `data_info.pages`.

In self-hosted layout detection, `PPDocLayoutDetector` also converts detected boxes and polygons from pixel coordinates into normalized 0-1000 coordinates.

The frontend/backend app, however, also has code that works with page/image pixel dimensions for display overlays, so its merged outputs can include page-size-aware boxes.
