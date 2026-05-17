"""Convert a PDF to markdown with stage-by-stage progress reporting."""

import argparse
import os
import sys
import time
from contextlib import contextmanager
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PDF = REPO_ROOT / "pdfs" / "Tricks of the 3D Game Programming Gurus.pdf"
DEFAULT_OUT = REPO_ROOT / "output"

# cd into marker-code so marker's settings finds local.env (GOOGLE_API_KEY, TORCH_DEVICE).
# Also load it ourselves so OPENAI_API_KEY (which marker doesn't auto-mirror) is visible.
os.chdir(REPO_ROOT / "marker-code")
from dotenv import load_dotenv  # noqa: E402
load_dotenv("local.env")


@contextmanager
def step(label):
    """Print a labeled stage with elapsed time and a clear divider."""
    bar = "-" * 60
    print(f"\n{bar}\n>>> {label}", flush=True)
    t0 = time.time()
    try:
        yield
    except BaseException:
        dt = time.time() - t0
        print(f"!!! {label} FAILED after {dt:.1f}s", flush=True)
        raise
    else:
        dt = time.time() - t0
        print(f"<<< {label} done in {dt:.1f}s", flush=True)


def sanity_check_openai(cfg: dict) -> None:
    """Make one tiny round-trip to OpenAI so we fail fast on bad keys / no credit."""
    import openai
    client = openai.OpenAI(api_key=cfg["openai_api_key"])
    r = client.chat.completions.create(
        model=cfg["openai_model"],
        messages=[{"role": "user", "content": "reply with the single word OK"}],
        max_tokens=10,
    )
    print(f"    openai reply: {r.choices[0].message.content.strip()[:40]!r}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pdf", nargs="?", default=str(DEFAULT_PDF))
    ap.add_argument("--output-dir", default=str(DEFAULT_OUT))
    ap.add_argument("--no-llm", action="store_true", help="Disable LLM processors")
    ap.add_argument(
        "--model",
        default="gpt-4o-mini",
        help="OpenAI model name (default: gpt-4o-mini)",
    )
    ap.add_argument("--page-range", default=None, help="e.g. '0-9' or '0,5-10'")
    ap.add_argument(
        "--full-vram",
        action="store_true",
        help="Skip the low-VRAM preset (use surya defaults). Default is low-VRAM (6 GB-safe).",
    )
    ap.add_argument("--layout-batch-size", type=int, default=None)
    ap.add_argument("--detection-batch-size", type=int, default=None)
    ap.add_argument("--recognition-batch-size", type=int, default=None)
    ap.add_argument("--table-rec-batch-size", type=int, default=None)
    ap.add_argument("--equation-batch-size", type=int, default=None)
    args = ap.parse_args()

    # Low-VRAM preset (default): shrink the biggest memory hogs to fit a 6 GB card.
    # Explicit per-flag overrides win; --full-vram disables the preset entirely.
    if not args.full_vram:
        args.layout_batch_size      = args.layout_batch_size      or 6
        args.detection_batch_size   = args.detection_batch_size   or 2
        args.recognition_batch_size = args.recognition_batch_size or 16
        args.table_rec_batch_size   = args.table_rec_batch_size   or 4
        args.equation_batch_size    = args.equation_batch_size    or 4

    pdf_path = Path(args.pdf).resolve()
    out_dir = Path(args.output_dir).resolve()
    if not pdf_path.exists():
        sys.exit(f"PDF not found: {pdf_path}")
    out_dir.mkdir(parents=True, exist_ok=True)

    llm_on = not args.no_llm
    print(f"PDF:    {pdf_path}")
    print(f"Output: {out_dir}")
    print(f"LLM:    {'OFF' if not llm_on else f'ON (openai/{args.model})'}")
    print(f"Pages:  {args.page_range or 'all'}")
    print(f"VRAM:   {'full (surya defaults)' if args.full_vram else 'low (6 GB preset)'}")

    llm_service_path = None
    llm_extra_cfg = {}
    if llm_on:
        key = os.environ.get("OPENAI_API_KEY", "")
        if not key:
            sys.exit("OPENAI_API_KEY not set (check marker-code/local.env)")
        llm_service_path = "marker.services.openai.OpenAIService"
        llm_extra_cfg["openai_api_key"] = key
        llm_extra_cfg["openai_model"] = args.model
        with step("Sanity-checking OpenAI"):
            sanity_check_openai(llm_extra_cfg)

    overall_t0 = time.time()

    with step("Loading 5 surya models into VRAM"):
        from marker.models import create_model_dict
        artifact_dict = create_model_dict()

    with step("Building config"):
        from marker.config.parser import ConfigParser
        cli_opts = {"use_llm": llm_on, "output_dir": str(out_dir)}
        if args.page_range:
            cli_opts["page_range"] = args.page_range
        for k in (
            "layout_batch_size",
            "detection_batch_size",
            "recognition_batch_size",
            "table_rec_batch_size",
            "equation_batch_size",
        ):
            v = getattr(args, k)
            if v is not None:
                cli_opts[k] = v
        cli_opts.update(llm_extra_cfg)
        cfg_parser = ConfigParser(cli_opts)
        config = cfg_parser.generate_config_dict()

    with step("Constructing PdfConverter (incl. LLM service)"):
        from marker.converters.pdf import PdfConverter
        chosen_service = llm_service_path if llm_on else None
        converter = PdfConverter(
            artifact_dict=artifact_dict,
            config=config,
            llm_service=chosen_service,
        )
        print(f"    processors: {len(converter.processor_list)}")
        if llm_on:
            print(f"    llm_service: {chosen_service}")

    with step(f"Resolving provider for {pdf_path.name}"):
        from marker.providers.registry import provider_from_filepath
        provider_cls = provider_from_filepath(str(pdf_path))
        print(f"    provider: {provider_cls.__name__}")

    with step("pdftext: extracting text from every page (CPU, can be slow)"):
        provider = provider_cls(str(pdf_path), converter.config)
        print(f"    pages loaded: {len(provider.page_lines)}")

    with step("Resolving builders (layout / line / ocr)"):
        from marker.builders.line import LineBuilder
        from marker.builders.ocr import OcrBuilder
        layout_builder = converter.resolve_dependencies(converter.layout_builder_class)
        line_builder = converter.resolve_dependencies(LineBuilder)
        ocr_builder = converter.resolve_dependencies(OcrBuilder)

    with step("DocumentBuilder (layout -> line detection -> ocr)"):
        from marker.builders.document import DocumentBuilder
        document = DocumentBuilder(converter.config)(
            provider, layout_builder, line_builder, ocr_builder
        )
        print(f"    document pages: {len(document.pages)}")

    with step("StructureBuilder (group captions / lists)"):
        from marker.builders.structure import StructureBuilder
        converter.resolve_dependencies(StructureBuilder)(document)

    total_procs = len(converter.processor_list)
    print(f"\n{'=' * 60}\nProcessor stage — {total_procs} processors\n{'=' * 60}")
    for i, processor in enumerate(converter.processor_list, 1):
        name = processor.__class__.__name__
        with step(f"[{i:2d}/{total_procs}] {name}"):
            processor(document)

    with step("Rendering markdown"):
        renderer = converter.resolve_dependencies(converter.renderer)
        rendered = renderer(document)

    with step(f"Saving to {out_dir}"):
        from marker.output import save_output
        basename = pdf_path.stem
        # Match marker_single CLI: drop everything in a per-document subfolder
        # so the .md, _meta.json, and figure JPEGs stay grouped together.
        doc_dir = out_dir / basename
        doc_dir.mkdir(parents=True, exist_ok=True)
        save_output(rendered, str(doc_dir), basename)
        print(f"    written under: {doc_dir}")

    total = time.time() - overall_t0
    h, rem = divmod(int(total), 3600)
    m, s = divmod(rem, 60)
    print(f"\n{'=' * 60}\nTOTAL: {h:02d}:{m:02d}:{s:02d}\n{'=' * 60}")


if __name__ == "__main__":
    main()
