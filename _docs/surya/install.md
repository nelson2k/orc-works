# Installation

From `repos-folder/surya/pyproject.toml`:

```
pip install surya-ocr
```

Requires Python 3.10+ and PyTorch ≥2.7. If you're on CPU you need to install the CPU build of torch first (see the upstream PyTorch install matrix).

## Runtime dependencies

```
transformers >=4.56.1
torch ^2.7.0
pydantic ^2.5.3
pydantic-settings ^2.1.0
python-dotenv ^1.0.0
pillow ^10.2.0
pypdfium2 ==4.30.0
filetype ^1.2.0
click ^8.1.8
platformdirs ^4.3.6
opencv-python-headless ==4.11.0.86
einops ^0.8.1
```

`pypdfium2` and `opencv-python-headless` are pinned to exact versions. Note `opencv-python-headless` (no GUI deps) is what surya uses — installing `opencv-python` alongside often causes conflicts.

## Optional groups

`[tool.poetry.group.xla]` — adds `torch-xla[tpu]` for Google Cloud TPU support. Activate with `poetry install --with xla`.

`[tool.poetry.group.dev]` — `jupyter`, `pytesseract` (benchmarks against tesseract), `pymupdf`, `datasets`, `rapidfuzz`, `streamlit`, `pytest`, `pdftext`, `tabulate`.

For the interactive Streamlit app:

```
pip install streamlit pdftext
surya_gui
```

For the Texify (LaTeX OCR) interactive app:

```
pip install streamlit==1.40 streamlit-drawable-canvas-jsretry
texify_gui
```

## Entry points

```
surya_detect    = surya.scripts.detect_text:detect_text_cli
surya_ocr       = surya.scripts.ocr_text:ocr_text_cli
surya_layout    = surya.scripts.detect_layout:detect_layout_cli
surya_gui       = surya.scripts.run_streamlit_app:streamlit_app_cli
surya_table     = surya.scripts.table_recognition:table_recognition_cli
surya_latex_ocr = surya.scripts.ocr_latex:ocr_latex_cli
texify_gui      = surya.scripts.run_texify_app:texify_app_cli
```

## From source

```
git clone https://github.com/VikParuchuri/surya.git
cd surya
poetry install
poetry shell
```

`poetry install --group dev` adds the benchmark/test dependencies. Surya benchmarks can also compare against tesseract — for that you also need `sudo apt-get install tesseract-ocr-all` and `TESSDATA_PREFIX` pointing at the data folder.

## Model downloads

Weights are pulled from `https://models.datalab.to` (S3 mirror) on first predictor call. Cached under `MODEL_CACHE_DIR`, which defaults to `$XDG_CACHE_HOME/datalab/models` (resolved via `platformdirs.user_cache_dir("datalab") / "models"`).

`PARALLEL_DOWNLOAD_WORKERS=10` controls concurrency of the initial fetch.
