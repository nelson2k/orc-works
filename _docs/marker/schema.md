# Schema (`marker/schema/`)

The schema layer is pydantic models for the document tree. The whole pipeline
operates on these classes.

## `BlockTypes` (`schema/__init__.py`)

A `str, Enum` listing every block type marker can emit. Includes both
leaf-ish types (`Text`, `Span`, `Char`, `Equation`, …) and *group* types
(`FigureGroup`, `TableGroup`, `ListGroup`, `PictureGroup`, `Page`,
`Document`).

## `Block` and `BlockId` (`schema/blocks/base.py`)

- `Block` — base pydantic model with `polygon: PolygonBox`, `page_id`,
  `structure: list[BlockId] | None`, `text_extraction_method`, format flags,
  removal flag, etc.
- `BlockId` — `{page_id, block_id, block_type}`; stringifies to
  `"/page/{p}/{Type}/{id}"` and is used as keys throughout (and in the JSON
  renderer `content-ref` placeholders).
- `BlockMetadata` — running counters for LLM call counts / tokens used /
  errors / "previous" text — aggregated per page by `aggregate_block_metadata`.
- `BlockOutput` — render-time wrapper carrying `html`, `polygon`, `id`,
  `children`, and `section_hierarchy`.

## Concrete blocks (`schema/blocks/`)

One file per block: `Caption`, `Code`, `Equation`, `Figure`, `Footnote`,
`Form`, `Handwriting`, `InlineMath` (`TextInlineMath`), `ListItem`,
`PageFooter`, `PageHeader`, `Picture`, `SectionHeader`, `Table`, `TableCell`,
`TableOfContents` (`toc.py`), `Text`, `ComplexRegion`, `Reference`. Each
defines its own `assemble_html` and any block-specific fields (e.g.
`Table.cells` / `TableCell.rowspan` / `Equation.html`).

## Groups (`schema/groups/`)

Groups own a `structure` list of child block ids and produce composite HTML:

| Class         | Wraps                                      |
|---------------|--------------------------------------------|
| `FigureGroup` | Figure + nearby Caption/Footnote           |
| `PictureGroup`| Picture + nearby Caption/Footnote          |
| `TableGroup`  | Table + nearby Caption/Footnote            |
| `ListGroup`   | Consecutive `ListItem`s                    |
| `PageGroup`   | Per-page: stores high/low-res images, layout flag, page polygon, and provides `add_block`, `get_block`, `contained_blocks`, `structure_blocks`, `render`. |

## Text level (`schema/text/`)

- `Line` — owns `Span`s; OCR result lands here.
- `Span` — owns characters / raw text + `formats` (plain / bold / italic /
  math / underline …) and optional `url`.
- `Char` (`char.py`) — only kept when `keep_chars=True` (used by `OCRConverter`).

## Polygon (`schema/polygon.py`)

`PolygonBox` is the unified geometry wrapper. Supports `from_bbox`, `bbox`,
`rescale`, `expand`, `fit_to_bounds`, `merge`, `minimum_gap`, etc. Everything
geometric in the pipeline is a `PolygonBox`.

## Registry (`schema/registry.py`)

`BLOCK_REGISTRY` maps `BlockTypes → dotted class path`. `get_block_class(t)`
returns the class; `register_block_class(t, cls)` overrides it (used by
`PdfConverter.override_map` to plug in custom block subclasses).

## Document (`schema/document.py`)

Top-level container. Holds `pages: list[PageGroup]`, `filepath`,
`table_of_contents`, `debug_data_path`. Exposes:

- `get_block(BlockId)`, `get_page(page_id)`, neighbour helpers
  (`get_next_block`, `get_prev_block`, `get_next_page`, `get_prev_page`).
- `contained_blocks(block_types)` — flat list across all pages.
- `render()` → `DocumentOutput(children, html)` for renderers to walk.
