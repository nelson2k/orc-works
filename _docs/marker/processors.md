# Processors (`marker/processors/`)

Processors mutate the `Document` in place after the builders are done. The
default order is set in `PdfConverter.default_processors`. Every processor
extends `BaseProcessor` and declares `block_types` it cares about.

LLM-backed processors live in `processors/llm/` — see
[processors_llm.md](processors_llm.md).

| Module               | Class                       | What it does |
|----------------------|-----------------------------|--------------|
| `order.py`           | `OrderProcessor`            | Re-sorts blocks within each page into reading order. |
| `block_relabel.py`   | `BlockRelabelProcessor`     | Allows `block_relabel` config to swap a block type for another (e.g. force `Text → SectionHeader`). |
| `line_merge.py`      | `LineMergeProcessor`        | Merges over-segmented lines (broken by detection) back together inside text blocks. |
| `blockquote.py`      | `BlockquoteProcessor`       | Detects indented runs of text and tags them as blockquotes. |
| `code.py`            | `CodeProcessor`             | Identifies code blocks (monospace font / indentation) and formats with fences. |
| `document_toc.py`    | `DocumentTOCProcessor`      | Builds `Document.table_of_contents` from `SectionHeader` blocks. |
| `equation.py`        | `EquationProcessor`         | Runs `texify` on `Equation` block crops to produce LaTeX. |
| `footnote.py`        | `FootnoteProcessor`         | Detects footnotes (often at page bottom) and links references. |
| `ignoretext.py`      | `IgnoreTextProcessor`       | Removes repeated boilerplate that appears on many pages. |
| `line_numbers.py`    | `LineNumbersProcessor`      | Strips legal-style line numbers from the left margin. |
| `list.py`            | `ListProcessor`             | Normalises bullet markers and list nesting inside `ListGroup`s. |
| `page_header.py`     | `PageHeaderProcessor`       | Removes / hides recurring page headers (kept if `keep_pageheader_in_output`). |
| `reference.py`       | `ReferenceProcessor`        | Builds `Reference` targets so cross-document links can be reconstructed. |
| `sectionheader.py`   | `SectionHeaderProcessor`    | Levels section headers (h1..h6) using font-size clustering across the doc. |
| `table.py`           | `TableProcessor`            | Re-OCRs tables with surya table-rec + recognition; emits `TableCell`s with rows / cols / spans. |
| `text.py`            | `TextProcessor`             | Cleans paragraph text (whitespace, hyphenation across lines, ligatures). |
| `blank_page.py`      | `BlankPageProcessor`        | Drops pages with no useful content. |
| `debug.py`           | `DebugProcessor`            | When `debug` mode is on, dumps per-page layout images, PDF images, and a block JSON dump into `debug_data_folder`. |

## Conventions

- Each processor reads its config from kwargs assigned by `assign_config`
  (e.g. `TableProcessor.row_split_threshold`).
- Heuristic processors read+modify blocks by walking
  `document.contained_blocks(self.block_types)` or by iterating pages.
- Order matters: e.g. `TableProcessor` must run before `LLMTableProcessor`,
  which in turn runs before `LLMTableMergeProcessor`.
