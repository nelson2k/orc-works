# LLM Processors (`marker/processors/llm/`)

Only run when `use_llm=True` and an `llm_service` is configured. Each one
sends image crops + prompts to the LLM and rewrites blocks based on the
structured response.

## Two base classes (`__init__.py`)

- `BaseLLMSimpleBlockProcessor` — per-block prompt that fits a single
  request/response. All "simple" processors share one `ThreadPoolExecutor` via
  `LLMSimpleBlockMetaProcessor` so their requests are interleaved and limited
  by `max_concurrency` (default 3). Each subclass implements
  `block_prompts(document)` (build prompts) and
  `rewrite_block(response, prompt_data, document)` (apply result).
- `BaseLLMComplexBlockProcessor` — runs its own thread pool for processors
  that need multi-step / multi-page logic (e.g. table merging).

`PromptData` (TypedDict) ships the prompt text, image, target block, response
schema, page, and a free-form `additional_data` dict.

## Processors

| Module                     | Class                              | Purpose |
|----------------------------|------------------------------------|---------|
| `llm_complex.py`           | `LLMComplexRegionProcessor`        | Re-formats `ComplexRegion` blocks (mixed layouts) into clean HTML. |
| `llm_equation.py`          | `LLMEquationProcessor`             | Improves LaTeX from texify by re-prompting on the equation image. |
| `llm_form.py`              | `LLMFormProcessor`                 | Extracts key/value pairs from `Form` blocks. |
| `llm_handwriting.py`       | `LLMHandwritingProcessor`          | Transcribes `Handwriting` blocks. |
| `llm_image_description.py` | `LLMImageDescriptionProcessor`     | When `disable_image_extraction`, replaces pictures with descriptions. |
| `llm_mathblock.py`         | `LLMMathBlockProcessor`            | Fixes inline math inside `TextInlineMath`. Pair with `redo_inline_math`. |
| `llm_sectionheader.py`     | `LLMSectionHeaderProcessor`        | Validates / re-levels `SectionHeader`s. |
| `llm_table.py`             | `LLMTableProcessor`                | Cleans up `Table` HTML (cell alignment, headers). |
| `llm_table_merge.py`       | `LLMTableMergeProcessor`           | Complex: merges multi-page tables (continuations) into one. |
| `llm_page_correction.py`   | `LLMPageCorrectionProcessor`       | Whole-page correction pass (`block_correction_prompt` CLI flag). |
| `llm_meta.py`              | `LLMSimpleBlockMetaProcessor`      | The fan-out wrapper that runs all simple LLM processors in parallel. |

## Wiring

`BaseConverter.initialize_processors` (see [converters.md](converters.md))
collects all `BaseLLMSimpleBlockProcessor` instances out of the configured
list, instantiates a single `LLMSimpleBlockMetaProcessor(processor_lst=…,
llm_service=…)`, and inserts it at the position of the last simple LLM
processor. Complex LLM processors stay where they were configured.
