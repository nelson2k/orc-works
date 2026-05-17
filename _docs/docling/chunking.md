# Chunking

[docling/chunking/__init__.py](../../repos-folder/docling/docling/chunking/__init__.py) is a thin re-export module. The actual chunkers live in `docling-core`. Behind the `feat-chunking` install extra (`docling-core[chunking]`).

```python
from docling.chunking import (
    BaseChunker, BaseChunk, BaseMeta,
    HierarchicalChunker, DocChunk, DocMeta,
    HybridChunker,
)
```

## `HierarchicalChunker`

Walks the `DoclingDocument` hierarchy (sections / subsections / paragraphs / lists / tables) and emits one `DocChunk` per leaf or grouped subtree. Each chunk carries:

- the chunk text,
- a `DocMeta` with the source document filename, the heading path (`["Chapter 1", "Section 1.2"]`), the page numbers, and the originating `DocItem` provenance.

Use this when the downstream embedder is tolerant of variable-length chunks and you want to keep semantic boundaries intact.

## `HybridChunker`

Wraps `HierarchicalChunker` and re-packs its output to target a desired token range using a tokenizer you provide. Splits oversized chunks and merges adjacent small ones to stay near a target token count. Required constructor args: `tokenizer` (anything that exposes `len(tokenizer.encode(text))`) and a `max_tokens` budget.

Typical RAG flow:

```python
from docling.document_converter import DocumentConverter
from docling.chunking import HybridChunker
from transformers import AutoTokenizer

converter = DocumentConverter()
result = converter.convert("paper.pdf")

tok = AutoTokenizer.from_pretrained("sentence-transformers/all-MiniLM-L6-v2")
chunker = HybridChunker(tokenizer=tok, max_tokens=512)
for chunk in chunker.chunk(result.document):
    text = chunk.text
    meta = chunk.meta            # DocMeta — pages, headings, provenance
    # embed(text), index alongside meta
```

## Why these aren't in the docling package itself

Chunking is purely structural — it only needs the `DoclingDocument` shape, not the converter or models. Keeping the implementation in `docling-core` lets downstream apps depend on chunking without pulling in PyTorch and the model zoo. `docling/chunking/__init__.py` re-exports the symbols so existing imports keep working.
