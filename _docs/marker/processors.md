# Processors

[marker/processors/](../../repos-folder/marker/marker/processors/). Each subclasses `BaseProcessor`, declares the block types it cares about, and mutates the `Document` in place. `PdfConverter` runs them in this order:

1. `OrderProcessor` — fix reading order for sliced/multi-column pages.
2. `BlockRelabelProcessor` — uses layout `top_k` to swap mislabeled blocks (e.g. Text → SectionHeader).
3. `LineMergeProcessor` — merge over-segmented lines back together.
4. `BlockquoteProcessor` — detect indented quote blocks.
5. `CodeProcessor` — heuristic code-block detection (monospace runs, indentation).
6. `DocumentTOCProcessor` — build the `table_of_contents` metadata from `SectionHeader` hierarchy.
7. `EquationProcessor` — crop `Equation` blocks and run texify/surya to get LaTeX.
8. `FootnoteProcessor` — anchor footnotes to the right superscripts.
9. `IgnoreTextProcessor` — drop repeated/junk text (page numbers, watermarks).
10. `LineNumbersProcessor` — strip left-margin line numbers (papers).
11. `ListProcessor` — clean up nested list rendering.
12. `PageHeaderProcessor` / `SectionHeaderProcessor` — fix levels and rendering.
13. `TableProcessor` — full table pipeline: surya table_rec for cell grid, pdftext or surya for cell contents.
14. **LLM processors** (only when `use_llm=True`) — `LLMTableProcessor`, `LLMTableMergeProcessor` (joins tables across pages), `LLMFormProcessor`, `LLMComplexRegionProcessor`, `LLMImageDescriptionProcessor`, `LLMEquationProcessor`, `LLMHandwritingProcessor`, `LLMMathBlockProcessor`, `LLMSectionHeaderProcessor`, `LLMPageCorrectionProcessor`. All inherit `BaseLLMProcessor` (in [processors/llm/__init__.py](../../repos-folder/marker/marker/processors/llm/__init__.py)) and get batched by `LLMSimpleBlockMetaProcessor` (`ThreadPoolExecutor`, default `max_concurrency=3`).
15. `TextProcessor` — paragraph joining, hyphenation across pages, final text cleanup.
16. `ReferenceProcessor` — link citations and references.
17. `BlankPageProcessor` — drop pages with no content.
18. `DebugProcessor` — when `debug=True`, writes layout/page images and JSON to `debug_data_folder`.

The order matters: layout fixes first, then per-block cleanup, then LLM passes, then global text passes, then debug.

## Authoring a custom processor

```python
from marker.processors import BaseProcessor
from marker.schema import BlockTypes

class MyProcessor(BaseProcessor):
    block_types = (BlockTypes.Text,)
    my_option: int = 5  # picked up from config via assign_config

    def __call__(self, document):
        for page in document.pages:
            for block in page.contained_blocks(document, self.block_types):
                ...
```

Pass it via CLI as `--processors mypkg.mymod.MyProcessor` (full path, comma-separated for several) or via Python by appending to `processor_list`.
