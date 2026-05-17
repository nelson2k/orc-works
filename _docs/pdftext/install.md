# Install

Python `>=3.10` (`pyproject.toml`). Runtime deps are tiny:

```
pypdfium2 == 4.30.0
pydantic >= 2.7
pydantic-settings >= 2.2
click >= 8.1
```

No torch, no numpy at runtime (numpy only loaded inside `tables.py` /
`pdf/utils.matrix_intersection_area`).

## pip

```bash
pip install pdftext
```

## From source (poetry)

```bash
git clone https://github.com/VikParuchuri/pdftext.git
cd pdftext
poetry install
```

The dev extras pull in pymupdf, pdfplumber, datasets, rapidfuzz, tabulate,
pillow, pytest — for benchmark comparison only.

## Entry point

`pyproject.toml`:

```toml
[tool.poetry.scripts]
pdftext = "pdftext.scripts.extract_text:extract_text_cli"
```

So `pdftext` becomes a CLI command after install. See [cli.md](cli.md).

## In this repo

pdftext is already pulled in transitively by `marker-pdf` in
`marker-code/venv/`. To use it standalone:

```bash
.\marker-code\venv\Scripts\Activate.ps1
pdftext "pdfs\Tricks of the 3D Game Programming Gurus.pdf" --out_path out.txt
```

(The version installed matches what marker's `PdfProvider` uses internally.)
