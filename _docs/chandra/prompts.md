# Prompts

[chandra/prompts.py](../../repos-folder/chandra/chandra/prompts.py). Two prompts ship with the package, picked via `BatchInputItem.prompt_type`:

| Key | Use |
|---|---|
| `"ocr_layout"` | Default for the CLI. Asks for HTML grouped into layout blocks with bboxes. |
| `"ocr"` | Plain OCR to HTML, no layout structuring. |

Callers can also pass a fully custom string via `BatchInputItem.prompt` — that bypasses `PROMPT_MAPPING`.

## Allowed tags / attributes

Both prompts end with a shared `PROMPT_ENDING` block that tells the model:

```
ALLOWED_TAGS = ["math", "br", "i", "b", "u", "del", "sup", "sub", "table", "tr",
                "td", "p", "th", "div", "pre", "h1", "h2", "h3", "h4", "h5",
                "ul", "ol", "li", "input", "a", "span", "img", "hr", "tbody",
                "small", "caption", "strong", "thead", "big", "code", "chem"]
ALLOWED_ATTRIBUTES = ["class", "colspan", "rowspan", "display", "checked",
                      "type", "border", "value", "style", "href", "alt",
                      "align", "data-bbox", "data-label"]
```

Plus a set of behavior rules:

- Inline math wrapped in `<math>…</math>`, KaTeX-compatible LaTeX. `display` attribute marks block math.
- Tables use `colspan` / `rowspan`.
- Images: detailed description in the alt attr and inside the surrounding div. Charts converted to high-fidelity data. Diagrams to mermaid.
- Forms: checkboxes and radio buttons marked.
- Text: paragraphs joined with `<p>`, `<br>` only when needed.
- Chemistry: `<chem>…</chem>` with reactive SMILES.
- Lists: indents preserved.

## Layout labels

The `ocr_layout` prompt enumerates the labels the model must use:

```
Caption, Footnote, Equation-Block, List-Group, Page-Header, Page-Footer,
Image, Section-Header, Table, Text, Complex-Block, Code-Block, Form,
Table-Of-Contents, Figure, Chemical-Block, Diagram, Bibliography, Blank-Page
```

These are the values that show up in `data-label` on the model's output `<div>`s and drive every post-processing branch.

Running `python -m chandra.prompts` (or `python chandra/prompts.py`) prints both prompts in full — useful when you want to sanity-check or tweak them.
