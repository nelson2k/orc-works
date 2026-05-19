# File Map

## Top-Level Summary

Top-level directories and approximate local size:

| Directory | Files | Approx. Size |
| --- | ---: | ---: |
| `.git` | 28 | 98.72 MB |
| `.github` | 7 | 0.01 MB |
| `apps` | 101 | 0.93 MB |
| `examples` | 188 | 96.41 MB |
| `glmocr` | 38 | 0.39 MB |
| `resources` | 7 | 11.81 MB |
| `skills` | 12 | 0.10 MB |

Common file types:

- 142 JPG images
- 92 Python files
- 27 Markdown files
- 19 TSX files
- 19 PNG images
- 15 TypeScript files
- 12 JSON files
- 11 YAML/YML files
- 2 CSS files
- 2 TOML files
- 2 shell scripts
- 1 font file
- 1 SVG file
- 1 HTML file
- 1 lock file

## Root Files

- `.gitignore`: ignore rules.
- `.pre-commit-config.yaml`: pre-commit checks.
- `agent.md`: agent-facing guide/context.
- `LICENSE`: Apache-2.0 license.
- `pyproject.toml`: SDK package metadata.
- `README.md`: English README.
- `README_zh.md`: Chinese README.

## `glmocr`

Main package files:

- `__init__.py`
- `__main__.py`
- `api.py`
- `cli.py`
- `config.py`
- `config.yaml`
- `maas_client.py`
- `ocr_client.py`
- `server.py`

Subdirectories:

- `dataloader`: `page_loader.py`
- `layout`: `base.py`, `layout_detector.py`
- `parser_result`: result classes.
- `pipeline`: orchestration, state, workers, tracker, helpers.
- `postprocess`: formatter and base post-processor.
- `tests`: unit/integration tests.
- `utils`: image, layout, markdown, result, visualization, lock, and logging helpers.

## `apps/backend`

Important files:

- `pyproject.toml`
- `Dockerfile`
- `README.md`
- `pytest.ini`
- `uv.lock`

Backend app areas:

- `app/main.py`: FastAPI app.
- `app/api`: task and system routes.
- `app/core`: worker/task manager/flow logic.
- `app/core/flows`: processing flow classes.
- `app/core/steps`: pipeline steps.
- `app/db`: database setup.
- `app/models`: task models.
- `app/repository`: task repository.
- `app/schemas`: response/task/system schemas.
- `app/utils`: config, logging, upload, image processing, converters.

## `apps/frontend`

Important files:

- `package.json`
- `pnpm-lock.yaml`
- `vite.config.ts`
- `tsconfig.json`
- `Dockerfile`
- `nginx.conf`
- `index.html`
- `components.json`

Frontend source:

- `src/main.tsx`
- `src/styles.css`
- `src/routeTree.gen.ts`
- `src/libs`: API and helper utilities.
- `src/store`: Zustand OCR store.
- `src/routes`: app routes.
- `src/routes/_ocr`: upload, preview, results, OCR page.
- `src/components/ocr`: PDF viewer, Markdown preview, JSON preview, overlay, copy button.
- `src/components/ui`: reusable UI components.
- `src/hooks`: PDF and Markdown virtualization/interaction hooks.

## `examples`

Important areas:

- `example.py`
- `source`: sample input files.
- `result`: generated sample outputs.
- `multi-gpu-deploy`: multi-GPU serving example.
- `self-host`: self-hosting README.
- `ollama-deploy`: Ollama README.
- `mlx-deploy`: MLX README.
- `finetune`: fine-tuning configs, README files, sample JSON/images.

## `resources`

Files:

- `WECHAT.md`
- `wechat.jpg`
- `speed.png`
- `realworld.png`
- `PingFang.ttf`
- `logo.svg`
- `docparse.png`

## `skills`

Files and folders:

- `sdk/SKILL.md`
- `glmocr/SKILL.md`
- `glmocr/scripts/glm_ocr_cli.py`
- `glmocr/scripts/config_setup.py`
- `glmocr/scripts/requirements.txt`
- `glmocr/references/output_schema.md`
- `glmocr-table/SKILL.md`
- `glmocr-table/scripts/glm_ocr_cli.py`
- `glmocr-formula/SKILL.md`
- `glmocr-formula/scripts/glm_ocr_cli.py`
- `glmocr-handwriting/SKILL.md`
- `glmocr-handwriting/scripts/glm_ocr_cli.py`

## Practical Interpretation

The folder is best understood as five layers:

1. SDK layer: `glmocr`.
2. App layer: `apps/backend` and `apps/frontend`.
3. Usage layer: `examples`.
4. Agent layer: `skills`.
5. Presentation/community layer: `resources`.

The SDK layer is compact. Most disk usage comes from `.git`, example outputs, and visual resources rather than Python source.
