import os
from dataclasses import dataclass
from pathlib import Path

from dotenv import load_dotenv

PROJECT_ROOT = Path(__file__).resolve().parents[2]
MARKER_ROOT = PROJECT_ROOT / "marker-code"

_ARTIFACT_DICT = None


@dataclass(frozen=True)
class ConversionOptions:
    pdf_path: Path
    output_root: Path
    use_llm: bool
    model: str
    page_range: str | None
    full_vram: bool


@dataclass(frozen=True)
class ConversionResult:
    document_name: str
    document_dir: Path
    markdown_file: Path
    metadata_file: Path
    markdown: str


def _load_models_once() -> dict:
    global _ARTIFACT_DICT
    if _ARTIFACT_DICT is None:
        from marker.models import create_model_dict

        _ARTIFACT_DICT = create_model_dict()
    return _ARTIFACT_DICT


def _build_config(options: ConversionOptions) -> dict:
    from marker.config.parser import ConfigParser

    cli_opts = {
        "use_llm": options.use_llm,
        "output_dir": str(options.output_root),
    }
    if options.page_range:
        cli_opts["page_range"] = options.page_range

    if not options.full_vram:
        cli_opts.update(
            {
                "layout_batch_size": 6,
                "detection_batch_size": 2,
                "recognition_batch_size": 16,
                "table_rec_batch_size": 4,
                "equation_batch_size": 4,
            }
        )

    if options.use_llm:
        openai_key = os.environ.get("OPENAI_API_KEY", "")
        if not openai_key:
            raise RuntimeError("OPENAI_API_KEY is not set in marker-code/local.env.")
        cli_opts["openai_api_key"] = openai_key
        cli_opts["openai_model"] = options.model

    return ConfigParser(cli_opts).generate_config_dict()


def convert_pdf(options: ConversionOptions) -> ConversionResult:
    if not options.pdf_path.exists():
        raise FileNotFoundError(f"PDF not found: {options.pdf_path}")

    load_dotenv(MARKER_ROOT / "local.env")
    os.chdir(MARKER_ROOT)

    from marker.converters.pdf import PdfConverter
    from marker.output import save_output

    options.output_root.mkdir(parents=True, exist_ok=True)
    config = _build_config(options)
    llm_service = "marker.services.openai.OpenAIService" if options.use_llm else None
    converter = PdfConverter(
        artifact_dict=_load_models_once(),
        config=config,
        llm_service=llm_service,
    )

    rendered = converter(str(options.pdf_path.resolve()))
    document_name = options.pdf_path.stem
    document_dir = options.output_root / document_name
    document_dir.mkdir(parents=True, exist_ok=True)
    save_output(rendered, str(document_dir), document_name)

    markdown_file = document_dir / f"{document_name}.md"
    metadata_file = document_dir / f"{document_name}_meta.json"
    return ConversionResult(
        document_name=document_name,
        document_dir=document_dir,
        markdown_file=markdown_file,
        metadata_file=metadata_file,
        markdown=markdown_file.read_text(encoding="utf-8"),
    )
