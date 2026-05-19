# Frontend App

## Purpose

The frontend under `apps/frontend` is a Vite/React app for uploading OCR inputs, previewing the uploaded file, polling backend task status, and displaying OCR results as Markdown or JSON.

The package is named `z-ocr-frontend`.

## Stack

The frontend uses:

- React 19
- TypeScript
- Vite
- TanStack Router
- Zustand
- axios
- react-pdf
- react-markdown
- KaTeX / remark-math / rehype-katex
- Radix UI primitives
- Tailwind CSS
- lucide-react icons
- Sonner notifications

Scripts:

- `dev`: Vite dev server on port 3006.
- `build`: Vite build then TypeScript compile.
- `preview`: Vite preview.
- `test`: Vitest.

## API Client

`src/libs/api.ts` defines the axios client.

Default API base:

```text
http://localhost:8000/api/v1
```

Override:

```text
VITE_API_URL
```

Main API functions:

- `uploadTask`: POST `/tasks/upload`.
- `getTaskStatus`: GET `/tasks/{taskId}`.

Upload sends:

- file
- `processing_mode=pipeline`
- optional `custom_url`

Polling expects statuses:

- `pending`
- `processing`
- `completed`
- `failed`

## Main Layout

`src/routes/_ocr/OCRPage.tsx` creates the primary UI:

- Left fixed upload sidebar.
- Center/left file preview panel.
- Right OCR results panel.

Local state:

- `uploadFile`
- `parsedResult`

`FileUpload` updates the uploaded file and task status. `FilePreview` displays the original document. `OCRResults` displays Markdown/JSON results.

## Upload Flow

`src/routes/_ocr/FileUpload.tsx` handles file selection and drag/drop.

Accepted MIME types:

- `image/png`
- `image/jpeg`
- `image/jpg`
- `application/pdf`

Accepted extensions:

- `.png`
- `.jpg`
- `.jpeg`
- `.pdf`

Max file size:

- 20 MB

After upload:

1. Calls `uploadTask`.
2. Receives `task_id`.
3. Starts polling every 2 seconds.
4. Stops polling when task status is `completed` or `failed`.

The UI shows a loading spinner while the task is active.

## Shared OCR State

`src/store/useOcrStore.ts` defines Zustand state:

- `hoveredBlockId`
- `clickedBlockId`
- `clickedPdfBlockId`
- `blocks`

Actions:

- `setHoveredBlockId`
- `setClickedBlockId`
- `setClickedPdfBlockId`
- `setBlocks`

This store is the bridge between the preview panel and the Markdown panel.

## Result Block Model

The frontend transforms backend `layout` entries into block objects:

- `id`
- `content`
- `bbox`
- `pageIndex`
- `isImage`
- `width`
- `height`

Blocks are used for:

- Markdown rendering.
- PDF/image overlay highlighting.
- Click/hover synchronization.
- Scrolling to selected regions.

## File Preview

`src/routes/_ocr/FilePreview.tsx` renders either:

- `PdfViewer` for PDFs.
- An `<img>` preview for images.

It calculates scale and offsets so OCR boxes can overlay the displayed file.

For PDFs, it uses original page dimensions from result metadata:

- `metadata.width`
- `metadata.height`

Fallback defaults:

- width: `1654`
- height: `2339`

Interactions:

- Hover/click preview blocks to highlight corresponding Markdown blocks.
- Hover/click Markdown blocks to highlight corresponding file regions.
- Scroll PDF preview to selected block.

## PDF Viewer

`src/components/ocr/PdfViewer.tsx` uses `react-pdf`.

Features:

- Virtual page rendering.
- Page buffer around visible viewport.
- Zoom in/out/reset.
- Page navigation.
- Current page input.
- Per-page overlay render hook.
- Mouse handlers per page.

The PDF worker and CMaps are configured from `registry.npmmirror.com` using the current `pdfjs.version`.

Virtual rendering defaults:

- buffer pages: 2
- placeholder height: 842

## OCR Results Panel

`src/routes/_ocr/OCRResults.tsx` renders:

- Markdown tab.
- JSON tab.
- Copy button for full Markdown.
- Download button for full Markdown.

Behavior:

- While pending/processing, shows loading state.
- On completed result with blocks, renders `MarkdownPreview`.
- On completed result with no content, shows empty state.
- On failed result, shows error message.

It transforms backend `layout` into store `blocks`.

## Markdown Preview

`src/components/ocr/MarkdownPreview.tsx` renders OCR blocks individually.

Features:

- Virtual block rendering.
- React Markdown.
- Raw HTML support through `rehype-raw`.
- Math support through `remark-math` and `rehype-katex`.
- Custom math-in-HTML plugin.
- Per-block highlighting.
- Copy button for active block.
- Scroll-to-block when a PDF block is clicked.

Virtual rendering defaults:

- buffer blocks: 10
- placeholder height: 100
- default block height: 150

## JSON Preview

`JsonPreview` is used by `OCRResults` to display the full backend response object when parsing is completed.

## User-Facing Language

Much of the frontend visible text is Chinese. Examples include upload instructions, loading state, empty states, and error states.

## Frontend Output Dependency

The frontend expects the backend task status response to include:

- `status`
- `full_markdown`
- `metadata`
- `layout`
- optional `images`

The highlighting system depends on each layout block having coordinates, page index, and a stable block id.
