# Backend Service

## Purpose

The backend under `apps/backend` is an async task service for document OCR. It exposes upload/status APIs, stores task metadata in SQLite through SQLAlchemy, runs worker loops, and writes OCR outputs to disk.

It is separate from the root SDK package. Its own `pyproject.toml` names it `glm-ocr`, version `0.1.0`, with Python `>=3.12`.

## Framework And Dependencies

The backend uses:

- FastAPI
- Uvicorn
- Pydantic / pydantic-settings
- SQLAlchemy async
- Alembic
- aiosqlite
- httpx
- aiofiles
- Pillow
- NumPy
- PyMuPDF
- pdf2image
- PyPDF2
- python-docx
- pytesseract
- psutil

## Application Entry

`apps/backend/app/main.py` creates the FastAPI app.

Startup lifecycle:

1. Initialize database.
2. Initialize task system.

Shutdown lifecycle:

1. Stop task system.
2. Close database connection.

Routes are registered under `/api/v1`:

- task routes from `app/api/tasks.py`
- system routes from `app/api/system.py`

Root endpoints:

- `GET /`: app name, version, status.
- `GET /health`: simple health response.

CORS allows all origins, methods, headers, and credentials.

## Settings

`apps/backend/app/utils/config.py` defines `Settings`.

Defaults:

- `APP_NAME`: `OCR Task System`
- `APP_VERSION`: `1.0.0`
- `HOST`: `0.0.0.0`
- `PORT`: `8000`
- `DATABASE_URL`: `sqlite+aiosqlite:///./tasks.db`
- `OUTPUT_DIR`: `./data`
- `WORKER_COUNT`: `5`
- `WORKER_POLL_INTERVAL`: `5`
- `TASK_TIMEOUT`: `3600`
- `MAX_CONCURRENT_TASKS`: `5`
- `DEFAULT_MAX_RETRIES`: `3`
- `CLEANUP_INTERVAL`: `300`
- `OLD_TASK_DAYS`: `30`
- `METRICS_INTERVAL`: `60`

Settings are loaded from `.env` and environment variables.

## Task API

Defined in `apps/backend/app/api/tasks.py`.

### POST `/api/v1/tasks/upload`

Accepts multipart form data:

- `file`: uploaded file.
- `processing_mode`: default `pipeline`.
- `priority`: default `2`.
- `custom_url`: optional.
- `output_format`: default `markdown`.

Behavior:

1. Generates `document_id` and `task_id`.
2. Saves uploaded file under `OUTPUT_DIR/<task_id>/`.
3. Builds optional OCR config from `custom_url`.
4. Submits a task to `TaskManager`.
5. Returns task id, document id, status, priority, and creation timestamp.

### GET `/api/v1/tasks/{task_id}`

Returns task status.

If `result_file_path` exists, it reads the JSON output and merges these fields into the response:

- `metadata`
- `full_markdown`
- `layout`

### DELETE `/api/v1/tasks/{task_id}`

Cancels a pending or processing task.

### GET `/api/v1/tasks/`

Lists tasks with optional status, limit, and offset.

### GET `/api/v1/tasks/file`

Reads a file by path.

For image MIME types, it returns binary image data directly. For other files, it returns metadata and decoded text content when possible.

## System API

Defined in `apps/backend/app/api/system.py`.

### GET `/api/v1/system/metrics`

Returns task counts and worker stats.

### GET `/api/v1/system/health`

Returns task manager health:

- status
- whether task manager is running
- worker count
- active worker count
- version

## Task Manager

`apps/backend/app/core/task_manager.py` owns the global task manager.

Responsibilities:

- Start workers.
- Recover tasks on startup.
- Submit task records.
- Fetch task status.
- Cancel tasks.
- List tasks.
- Build metrics.
- Run monitoring loop for expired lock recovery.
- Run cleanup loop for old tasks.

It creates `Worker` instances according to `WORKER_COUNT`.

## Worker Lifecycle

`apps/backend/app/core/worker.py` defines `Worker`.

Worker behavior:

1. Poll pending tasks.
2. Acquire a lock through `LockManager`.
3. Build `ProcessingContext`.
4. Create a flow with `FlowFactory`.
5. Execute the flow.
6. Release lock and mark task completed or failed.
7. Retry failures through `RetryHandler` when appropriate.

Worker metrics track:

- tasks started
- tasks completed
- tasks failed
- total execution time
- average execution time

## Pipeline Flow

`apps/backend/app/core/flows/pipeline_flow.py` defines `PipelineFlow`.

It has three weighted steps:

- `pdf_to_image`: 0-20%.
- `layout_and_ocr`: 20-85%.
- `result_merge`: 85-100%.

The flow prepares `OUTPUT_DIR/<task_id>/`, runs each step, updates progress, and returns output paths.

## Backend OCR Step

`apps/backend/app/core/steps/layout_ocr.py` calls `LayoutAndOCRClient` to process each page image.

For each page:

1. Calls OCR service.
2. Iterates returned layout blocks.
3. Converts VLM bboxes to page coordinates.
4. Crops image blocks when `label == "image"`.
5. Builds page layout blocks with content, bbox, index, image path, and page index.
6. Saves `ocr_result.json`.

The result contains:

- `success`
- `pages`
- `total_pages`
- `images_dir`
- `ocr_result_file`
- `ref_image_paths`

## Backend Output Shape

The backend task status endpoint expects merged result JSON to contain at least:

- `metadata`
- `full_markdown`
- `layout`

The frontend consumes those fields to render Markdown, JSON, and overlay boxes.

## Backend And SDK Relationship

The backend is not just a thin wrapper around `glmocr.GlmOcr`. It has its own task pipeline under `apps/backend/app/core`, with upload handling, workers, DB state, PDF conversion, OCR service calls, and result merging.

The SDK and backend share the broad OCR/document parsing domain, but their code paths are distinct.
