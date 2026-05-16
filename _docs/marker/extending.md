# Extending Marker

Marker's components are wired together by `BaseConverter.resolve_dependencies`,
which inspects each class's `__init__` signature and pulls values from
`self.artifact_dict` or `self.config`. This means most extensions are just
"write a class, give it the right `__init__` args."

## Add a custom processor

```python
from marker.processors import BaseProcessor
from marker.schema import BlockTypes

class StripDraftWatermark(BaseProcessor):
    block_types = (BlockTypes.Text,)

    def __call__(self, document):
        for block in document.contained_blocks(self.block_types):
            for line in block.contained_blocks(document, [BlockTypes.Line]):
                ...
```

Plug it in:

```bash
marker_single doc.pdf \
  --processors marker.processors.order.OrderProcessor,my.pkg.StripDraftWatermark
```

Or pass `processor_list=[StripDraftWatermark]` directly to `PdfConverter`.

## Override a block class

```python
from marker.schema.blocks import Table
class MyTable(Table):
    def assemble_html(self, document, child_blocks, parent_structure):
        return f"<div class='my-table'>{super().assemble_html(...)}</div>"
```

Then either subclass `PdfConverter` and set `override_map={BlockTypes.Table: MyTable}`
or call `marker.schema.registry.register_block_class(BlockTypes.Table, MyTable)`
before instantiating the converter.

## Add a renderer

Subclass `BaseRenderer`, implement `__call__(document)`, return any pydantic
model. Pass `renderer=MyRenderer` to the converter (or
`--converter_cls`+`--processors`+`renderer="my.module.MyRenderer"`).

If you want it selectable via `--output_format`, extend `ConfigParser.get_renderer`
in your fork.

## Add a provider

Subclass `BaseProvider`, implement the four `get_*` methods. Then register the
file-type sniffing rule in `providers/registry.py` (or pre-pick the class and
instantiate your converter manually, skipping `provider_from_filepath`).

## Add an LLM service

Subclass `BaseService`. Implement `process_images` (encoding) and `__call__`
(prompt → response_schema → dict). Pick it via:

```bash
--use_llm --llm_service my.pkg.MyLLMService --my_required_key ...
```

`ConfigCrawler` will pick up any `Annotated[...]` class attributes and surface
them in `config --help`.

## Tips

- Use `BlockTypes` for any block-type literal — strings will silently miss
  registry lookups.
- `Annotated[type, "help text"]` on class attributes is the supported pattern
  for adding new tunables; `assign_config` will fill them from CLI / JSON.
- For required config (e.g. API keys), declare them as `Annotated[...] = None`;
  `verify_config_keys` will refuse to construct the object until they are set.
