# Prompts (`chandra/prompts.py`)

Two prompts, both ending with the same `PROMPT_ENDING` that locks down which
HTML tags and attributes the model is allowed to use.

## `OCR_LAYOUT_PROMPT` — the default

> OCR this image to HTML, arranged as layout blocks. Each layout block should
> be a div with the data-bbox attribute representing the bounding box of the
> block in `x0 y0 x1 y1` format. Bboxes are normalized 0-1000. The data-label
> attribute is the label for the block.

Allowed `data-label` values:

| Label              | Notes                                            |
|--------------------|--------------------------------------------------|
| `Caption`          | Figure/table captions.                           |
| `Footnote`         |                                                  |
| `Equation-Block`   | Math set apart, rendered with `<math display>`.  |
| `List-Group`       | A whole list (`<ul>`/`<ol>`).                    |
| `Page-Header`      | Stripped by default in markdown/html output.     |
| `Page-Footer`      | Stripped by default in markdown/html output.     |
| `Image`            | Cropped from the page image at this bbox.        |
| `Section-Header`   | Maps to `<h1>`..`<h5>`.                          |
| `Table`            | Always rendered as raw HTML.                     |
| `Text`             | Paragraph. Wrapped in `<p>` if missing tags.     |
| `Complex-Block`    | Mixed-layout fallback.                           |
| `Code-Block`       |                                                  |
| `Form`             | Includes `<input>` for checkboxes/radio.         |
| `Table-Of-Contents`|                                                  |
| `Figure`           | Like Image, but for charts/diagrams w/ caption.  |
| `Chemical-Block`   | Uses `<chem>` for SMILES.                        |
| `Diagram`          | Model is told to convert to **mermaid**.         |
| `Bibliography`     |                                                  |
| `Blank-Page`       | Whole-page sentinel; dropped from output.        |

## `OCR_PROMPT` — plain

Same `PROMPT_ENDING`, but no layout-block requirement; the model emits
free-form HTML. Use when you don't care about block-level bboxes (e.g.
single-block crops).

## `PROMPT_ENDING` — the contract

Restricts the model to these tags:
`math, br, i, b, u, del, sup, sub, table, tr, td, p, th, div, pre, h1-h5, ul,
ol, li, input, a, span, img, hr, tbody, small, caption, strong, thead, big,
code, chem`

And these attributes:
`class, colspan, rowspan, display, checked, type, border, value, style, href,
alt, align, data-bbox, data-label`

Key behavioural rules baked in:

- **Math** in `<math>...</math>`, KaTeX-compatible LaTeX, `display` attr for
  block math.
- **Tables** must use `colspan`/`rowspan` to preserve real structure.
- **Images**: description goes in the `alt` attr (and inside the div); the
  model is told *not* to fill `src` (post-processor wires it). Charts go to
  high-fidelity tabular data, diagrams to mermaid.
- **Chemistry** in `<chem>` with reactive SMILES.
- **Forms**: checkboxes/radios rendered with `<input type="checkbox" checked>`.
- **Reading order** should be correct and natural.

## Selecting a prompt

```python
PROMPT_MAPPING = {"ocr_layout": OCR_LAYOUT_PROMPT, "ocr": OCR_PROMPT}
```

Override at the call site by setting `BatchInputItem.prompt=<string>` — that
wins over `prompt_type`.

## Inspecting the prompts

```bash
python -m chandra.prompts
```

Prints both prompts to stdout.
