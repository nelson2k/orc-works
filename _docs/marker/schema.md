# Schema

[marker/schema/](../../repos-folder/marker/marker/schema/). Pydantic models for the in-memory document tree.

## `BlockTypes`

[schema/__init__.py](../../repos-folder/marker/marker/schema/__init__.py). A string-valued `Enum`. The full list:

```
Line Span Char
FigureGroup TableGroup ListGroup PictureGroup
Page Caption Code Figure Footnote Form Equation Handwriting
TextInlineMath ListItem PageFooter PageHeader Picture
SectionHeader Table Text TableOfContents Document
ComplexRegion TableCell Reference
```

The layout model emits a subset (the typed regions); the others are added by builders/processors.

## `Document`

[schema/document.py](../../repos-folder/marker/marker/schema/document.py). Holds:

- `filepath: str`
- `pages: List[PageGroup]`
- `table_of_contents: List[TocItem] | None`
- `debug_data_path: str | None`

Helper methods: `get_block(BlockId)`, `get_page(page_id)`, `get_next_block`, `get_prev_block`, `contained_blocks(types)`, `render(block_config)`.

## `Block` (base)

[schema/blocks/base.py](../../repos-folder/marker/marker/schema/blocks/base.py). All concrete blocks subclass this.

- `polygon: PolygonBox` — 4-corner box.
- `block_type`, `block_id`, `page_id`.
- `text_extraction_method: "pdftext" | "surya" | "gemini"`.
- `structure: List[BlockId] | None` — ordered child block ids.
- `top_k: Dict[BlockTypes, float] | None` — layout confidence.
- `metadata: BlockMetadata` — `llm_request_count`, `llm_tokens_used`, etc. — aggregates upward for stats.
- `lowres_image` / `highres_image` — only on `PageGroup`.
- `ignore_for_output`, `replace_output_newlines`.

`BlockId.__str__` is `/page/{page_id}/{type}/{block_id}` — that's the format used in JSON output and in `<content-ref src=…>` placeholders during rendering.

## `PageGroup`

[schema/groups/page.py](../../repos-folder/marker/marker/schema/groups/page.py). One per page. Owns the page image and is the only thing with `add_block(cls, polygon)`, `add_full_block(block)`, and `merge_blocks(provider_lines, …)`.

## Concrete block dirs

- [schema/blocks/](../../repos-folder/marker/marker/schema/blocks/) — `text.py`, `table.py`, `equation.py`, `figure.py`, `code.py`, `sectionheader.py`, `caption.py`, `footnote.py`, `form.py`, `inlinemath.py`, `listitem.py`, `handwriting.py`, `picture.py`, `reference.py`, `tablecell.py`, `pageheader.py`, `pagefooter.py`, `toc.py`, `complexregion.py`, `basetable.py`. Each declares the HTML template it renders to.
- [schema/groups/](../../repos-folder/marker/marker/schema/groups/) — `figure.py`, `table.py`, `picture.py`, `list.py`, `page.py`. Wrappers added by `StructureBuilder` so captions stay attached.
- [schema/text/](../../repos-folder/marker/marker/schema/text/) — `Line`, `Span`, `Char`. The text-layer primitives.

## `BlockOutput`

What renderers consume: `{html, polygon, id, children, section_hierarchy}`. `children` are recursively `BlockOutput`s. Rendering replaces every `<content-ref src=ID>` with the rendered child HTML.
