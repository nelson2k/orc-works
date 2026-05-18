# Package layout

Contents of `repos-folder/MinerU/`:

```
MinerU/
├── pyproject.toml             setuptools build + extras (vlm, vllm, lmdeploy, mlx, pipeline, gradio, core, all, test)
├── mineru.template.json       Template for ~/mineru.json
├── update_version.py          Maintenance script
├── README.md / README_zh-CN.md / SECURITY.md / MinerU_CLA.md / LICENSE.md
├── mkdocs.yml                 Docs site config
│
├── mineru/                    The Python package
│   ├── __init__.py
│   ├── version.py             __version__
│   │
│   ├── cli/                   Console scripts + FastAPI server
│   │   ├── client.py          `mineru`  — orchestrating client
│   │   ├── fast_api.py        `mineru-api`  — FastAPI parse server
│   │   ├── router.py          `mineru-router`  — multi-server router
│   │   ├── gradio_app.py      `mineru-gradio`  — browser UI
│   │   ├── models_download.py `mineru-models-download`
│   │   ├── vlm_server.py      `mineru-vllm-server` / `mineru-lmdeploy-server` / `mineru-openai-server`
│   │   ├── api_client.py      HTTP client shared by `mineru` and `mineru-router`
│   │   ├── api_protocol.py    Shared protocol constants (DEFAULT_MAX_CONCURRENT_REQUESTS, ...)
│   │   ├── common.py          `do_parse` / `aio_do_parse` / `read_fn` — library entry points
│   │   ├── output_paths.py    `resolve_parse_dir(...)`
│   │   ├── public_http_client_policy.py   Public-bind safety policy
│   │   ├── visualization.py   layout.pdf / span.pdf debug renderer
│   │   ├── vlm_preload.py     VLM model preload helpers
│   │   └── ...
│   │
│   ├── backend/               The analyze passes
│   │   ├── pipeline/          Pipeline backend (layout + OCR + formula + table)
│   │   ├── vlm/               VLM backend (model_output_to_middle_json, vlm_analyze, ...)
│   │   ├── hybrid/            Hybrid backend (pipeline-anchored VLM)
│   │   └── office/            Native DOCX / PPTX / XLSX parsing
│   │
│   ├── model/                 Model implementations
│   │   ├── layout/            pp_doclayoutv2
│   │   ├── ocr/               pytorch_paddle, seal_crop, seal_det_warp
│   │   ├── mfr/               unimernet, pp_formulanet_plus_m
│   │   ├── table/             cls, rec
│   │   ├── vlm/               vllm_server, lmdeploy_server
│   │   └── utils/             pytorchocr (PaddleOCR-derived torch code), tools (ckpt conversion)
│   │
│   ├── data/                  IO abstractions
│   │   ├── data_reader_writer/    DataReader/Writer, FileBased, S3, MultiBucketS3, Dummy
│   │   ├── io/                    Lower-level transport (http, s3)
│   │   └── utils/                 Misc data helpers
│   │
│   ├── utils/                 Helper layer (bbox, char, draw, pdf*, ocr_utils, ...)
│   └── resources/             Shipped assets (fonts, configs)
│
├── projects/                  Auxiliary sub-projects
├── tests/                     Pytest suite
├── docker/                    Reference Dockerfiles per backend
└── docs/                      MkDocs sources
```

## Module identity

```
name    = "mineru"
license = "LicenseRef-MinerU-Open-Source-License"   (custom Apache-2.0-based)
python  = ">=3.10,<3.14"
```

## Major dependencies

| Group | Modules |
|---|---|
| Core IO | `boto3`, `httpx`, `requests`, `python-multipart`, `fastapi`, `uvicorn` |
| PDF | `pypdfium2`, `pypdf`, `pdfminer.six`, `reportlab` |
| Imaging | `pillow`, `opencv-python`, `scikit-image`, `numpy` |
| Sniffing | `magika` (mime), `fast-langdetect` (language) |
| Office formats | `python-docx`, `pypptx-with-oxml`, `mammoth`, `openpyxl`, `pandas`, `beautifulsoup4`, `lxml` |
| LaTeX | `pylatexenc` |
| Repair | `json-repair` |
| Logging / CLI | `loguru`, `click`, `tqdm` |
| Model registry | `huggingface-hub`, `modelscope` |
| LLM / VLM | `openai`, `mineru-vl-utils`, `qwen-vl-utils` |
| Pipeline extra | `torch`, `torchvision`, `transformers`, `onnxruntime`, `albumentations`, `shapely`, `pyclipper`, `omegaconf`, `dill`, `PyYAML`, `ftfy` |
| VLM extras | `accelerate`; engine of choice: `vllm` / `lmdeploy` / `mlx-vlm` |
| UI | `gradio`, `gradio-pdf` |

## CLI declarations (`pyproject.toml [project.scripts]`)

```toml
mineru                 = "mineru.cli.client:main"
mineru-vllm-server     = "mineru.cli.vlm_server:vllm_server"
mineru-lmdeploy-server = "mineru.cli.vlm_server:lmdeploy_server"
mineru-openai-server   = "mineru.cli.vlm_server:openai_server"
mineru-models-download = "mineru.cli.models_download:download_models"
mineru-api             = "mineru.cli.fast_api:main"
mineru-router          = "mineru.cli.router:main"
mineru-gradio          = "mineru.cli.gradio_app:main"
```
