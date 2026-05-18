# Frontend

The web UI lives in [web-app/](../../repos-folder/paperless-gpt/web-app/). Stack:

- React 18 + TypeScript
- Vite as the bundler / dev server
- Tailwind CSS
- ESLint, Playwright (E2E tests in `web-app/e2e/`)

The built `web-app/dist/` is embedded into the Go binary via [embedded_assets.go](../../repos-folder/paperless-gpt/embedded_assets.go). At runtime, paperless-gpt prefers `web-app/dist/` from disk if present (useful for `vite dev` against a local Go build), otherwise serves the embedded copy.

## Routes

Single-page app with client-side routing. Five routes; all 200 the same `index.html` server-side so the React router takes over:

| Route | Component | Purpose |
|---|---|---|
| `/` | `App.tsx` shell with `DocumentProcessor.tsx` | The main review queue — lists documents with the `MANUAL_TAG` and runs LLM suggestions on them |
| `/experimental-ocr` | `ExperimentalOCR.tsx` | Per-document OCR runner — submit jobs, view per-page progress, re-OCR individual pages |
| `/history` | `History.tsx` | Paginated list of modifications paperless-gpt made; undo button per row |
| `/settings` | (Settings) | Edit prompts; toggle custom-fields generation and pick fields |
| `/adhoc-analysis` | `AdhocAnalysis.tsx` | Multi-document free-form prompt — concatenates content of selected docs, runs against the LLM |

## Files in [web-app/src/](../../repos-folder/paperless-gpt/web-app/src/)

- `main.tsx` — Vite entry point; mounts `<App />` to `#root`.
- `App.tsx` — top-level layout + react-router routes.
- `App.css`, `index.css`, `tailwind.config.js`, `postcss.config.js` — styling.
- `DocumentProcessor.tsx` — the review queue. Polls `/api/documents`, lets the user select fields to generate, displays diffs.
- `ExperimentalOCR.tsx` — OCR job submission and review. Polls `/api/jobs/ocr/:job_id` for live `PagesDone/TotalPages`. Per-page re-OCR.
- `AdhocAnalysis.tsx` — multi-document analysis form. Multi-select documents, free-form prompt input, calls `/api/analyze-documents`.
- `History.tsx` — pagination over `/api/modifications`, undo via `/api/undo-modification/:id`.
- `ocrStatus.ts` — small helpers around polling job status from `/api/jobs/ocr/:job_id`.
- `components/` — shared widgets (`SettingsPage`, prompt editor, etc.).
- `vite-env.d.ts` — Vite's ambient module declarations.

## Build

`npm install` then `npm run build` in `web-app/` produces `web-app/dist/`. The Dockerfile does this in a build stage, then copies the output into the Go binary build context so `embedded_assets.go` can pick it up via `go:embed`.

For local dev: run the Go binary (which serves `/api/*`) and `npm run dev` in `web-app/` simultaneously; Vite's dev server proxies API calls to the Go server. The relevant config is in `web-app/vite.config.ts`.

## Tests

E2E with Playwright. Config in `web-app/playwright.config.ts`, specs in `web-app/e2e/`, plus a `docker-compose.test.yml` that stands up paperless-ngx + paperless-gpt + a test runner. Test-fixture quirks and known failure modes are documented in [E2E_TEST_FIXES.md](../../repos-folder/paperless-gpt/E2E_TEST_FIXES.md).

## API surface used by the UI

Every backend endpoint listed in the http-api notes is consumed by the UI. The polling cadence is mostly 1–2 s for in-flight job status and 5–10 s for queue refreshes. There are no WebSockets — everything is plain GET polling.
