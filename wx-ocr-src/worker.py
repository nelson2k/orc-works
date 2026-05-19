import sys
import os
import io
import re
import gc
import json
import base64
import threading
import subprocess
import time
import traceback


# ---- Protocol stdout isolation ----------------------------------------------
# Native libraries (MuPDF, libcms, etc.) sometimes printf warnings/errors
# straight to FD 1, which would corrupt the JSON protocol we share with the
# parent. Dup FD 1 into a private FD for our protocol, then redirect FD 1
# itself (and Python's sys.stdout) at NUL so any leak is swallowed.
_proto_fd = os.dup(1)
_proto_stream = os.fdopen(_proto_fd, "w", buffering=1, encoding="utf-8", newline="\n")

_devnull_w_fd = os.open(os.devnull, os.O_WRONLY)
os.dup2(_devnull_w_fd, 1)
os.close(_devnull_w_fd)
sys.stdout = open(os.devnull, "w", buffering=1, encoding="utf-8")


REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUTPUT_ROOT = os.path.join(REPO_ROOT, "output")
REPOS_FOLDER = os.path.join(REPO_ROOT, "repos-folder")
_FS_UNSAFE = re.compile(r'[<>:"/\\|?*]')


_send_lock = threading.Lock()


def send(msg):
    line = json.dumps(msg) + "\n"
    with _send_lock:
        _proto_stream.write(line)
        _proto_stream.flush()


def send_stage(name):
    send({"type": "progress", "kind": "stage", "name": name})


# ---- Remote metrics streaming ----------------------------------------------
# A daemon thread emits type:"metrics" events every second while a request is
# being processed. The parent GUI prefers these over its locally-collected
# samples when running in Remote mode, so the bars show the 4070 instead of
# the user's Windows laptop. Between requests the thread sleeps — pausing
# emission avoids back-pressuring the pipe when the GUI isn't reading.

_metrics_active = threading.Event()
_metrics_thread_started = False


def _gpu_metrics_via_nvidia_smi():
    try:
        out = subprocess.check_output(
            [
                "nvidia-smi",
                "--query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu",
                "--format=csv,noheader,nounits",
            ],
            stderr=subprocess.DEVNULL,
            timeout=2,
        ).decode("ascii", errors="ignore").strip()
    except Exception:
        return None
    line = out.splitlines()[0] if out else ""
    parts = [p.strip() for p in line.split(",")]
    if len(parts) < 4:
        return None
    try:
        return (float(parts[0]), float(parts[1]), float(parts[2]), float(parts[3]))
    except ValueError:
        return None


def _emit_metrics_sample():
    import psutil
    cpu = psutil.cpu_percent(interval=None)
    vm = psutil.virtual_memory()
    msg = {
        "type": "metrics",
        "cpu_pct": cpu,
        "ram_pct": vm.percent,
        "ram_used_gb": (vm.total - vm.available) / (1024.0 ** 3),
        "has_gpu": False,
    }
    gpu = _gpu_metrics_via_nvidia_smi()
    if gpu is not None:
        gpu_pct, vram_used_mb, vram_total_mb, temp_c = gpu
        msg["has_gpu"] = True
        msg["gpu_pct"] = gpu_pct
        msg["vram_used_mb"] = vram_used_mb
        msg["vram_total_mb"] = vram_total_mb
        msg["vram_pct"] = (100.0 * vram_used_mb / vram_total_mb) if vram_total_mb > 0 else 0.0
        msg["temp_c"] = temp_c
    send(msg)


def _metrics_loop():
    try:
        import psutil
        psutil.cpu_percent(interval=None)  # prime the delta counter
    except Exception:
        return
    while True:
        # Block until a request is in flight; sleeping here means no events
        # leak into the pipe while the GUI is idle.
        _metrics_active.wait()
        try:
            _emit_metrics_sample()
        except Exception:
            pass
        # 1 Hz cadence. Re-check _metrics_active at the top of the loop so a
        # request that completed mid-sleep doesn't get a trailing sample.
        time.sleep(1.0)


def _start_metrics_thread_once():
    global _metrics_thread_started
    if _metrics_thread_started:
        return
    _metrics_thread_started = True
    t = threading.Thread(target=_metrics_loop, name="metrics", daemon=True)
    t.start()


# Patch tqdm BEFORE marker/surya import it, so progress bars become JSON events.
import tqdm as _tqdm_pkg
import tqdm.auto as _tqdm_auto

_OrigTqdm = _tqdm_pkg.tqdm
_devnull = open(os.devnull, "w")


def _emit_tqdm(t, event):
    try:
        send({
            "type": "progress",
            "kind": "tqdm",
            "event": event,
            "desc": getattr(t, "desc", "") or "",
            "n": getattr(t, "n", 0),
            "total": getattr(t, "total", None) or 0,
        })
    except Exception:
        pass


class _EventTqdm(_OrigTqdm):
    def __init__(self, *args, **kwargs):
        kwargs["file"] = _devnull
        kwargs.setdefault("leave", False)
        super().__init__(*args, **kwargs)
        _emit_tqdm(self, "start")

    def update(self, n=1):
        ret = super().update(n)
        _emit_tqdm(self, "tick")
        return ret

    def close(self):
        ret = super().close()
        _emit_tqdm(self, "end")
        return ret


_tqdm_pkg.tqdm = _EventTqdm
_tqdm_auto.tqdm = _EventTqdm


import pymupdf  # noqa: E402


# Resident model state. Only one of {_marker_models, _vlm} should be loaded
# at a time on a low-VRAM card; the loaders defensively unload the other.
_marker_models = None
_vlm = None


def _empty_cuda_cache():
    try:
        import torch
        if torch.cuda.is_available():
            torch.cuda.empty_cache()
    except Exception:
        pass


def _unload_marker():
    global _marker_models
    if _marker_models is None:
        return
    send_stage("unloading_marker")
    _marker_models = None
    gc.collect()
    _empty_cuda_cache()


def _unload_vlm():
    global _vlm
    if _vlm is None:
        return
    send_stage("unloading_vlm")
    _vlm = None
    gc.collect()
    _empty_cuda_cache()


def render(path, page, dpi):
    doc = pymupdf.open(path)
    try:
        pages = len(doc)
        if page < 0 or page >= pages:
            return {
                "type": "error",
                "message": f"page {page} out of range (doc has {pages} pages)",
            }
        pix = doc[page].get_pixmap(dpi=dpi)
        png = pix.tobytes(output="png")
        return {
            "type": "image",
            "page": page,
            "pages": pages,
            "png_base64": base64.b64encode(png).decode("ascii"),
        }
    finally:
        doc.close()


def _ensure_marker():
    global _marker_models
    if _marker_models is None:
        _unload_vlm()
        send_stage("loading_models")
        from marker.models import create_model_dict
        _marker_models = create_model_dict()
    return _marker_models


_BLOCK_COLORS = {
    "Text": (60, 130, 220),
    "TextInlineMath": (60, 130, 220),
    "SectionHeader": (220, 60, 60),
    "Picture": (220, 140, 80),
    "Figure": (220, 140, 80),
    "Table": (60, 180, 100),
    "TableCell": (90, 200, 130),
    "Form": (200, 110, 200),
    "Equation": (180, 100, 220),
    "Code": (110, 110, 110),
    "Caption": (140, 200, 100),
    "Footnote": (150, 150, 150),
    "ListItem": (210, 200, 70),
    "PageHeader": (180, 180, 180),
    "PageFooter": (180, 180, 180),
    "TableOfContents": (90, 180, 200),
    "ComplexRegion": (180, 60, 130),
}


def _emit_layout_overlay(document, page_idx=0):
    from PIL import ImageDraw

    page_group = document.pages[page_idx]
    base_img = page_group.get_image(highres=True).copy().convert("RGB")

    page_w, page_h = page_group.polygon.size
    img_w, img_h = base_img.size
    if page_w <= 0 or page_h <= 0:
        return
    sx = img_w / page_w
    sy = img_h / page_h

    draw = ImageDraw.Draw(base_img)
    for block in page_group.structure_blocks(document):
        bbox = block.polygon.bbox
        x0, y0, x1, y1 = bbox[0] * sx, bbox[1] * sy, bbox[2] * sx, bbox[3] * sy
        type_name = getattr(block.block_type, "name", str(block.block_type))
        color = _BLOCK_COLORS.get(type_name, (128, 128, 128))
        draw.rectangle([x0, y0, x1, y1], outline=color, width=4)

    buf = io.BytesIO()
    base_img.save(buf, format="PNG")
    send({
        "type": "progress",
        "kind": "image",
        "page": page_group.page_id,
        "png_base64": base64.b64encode(buf.getvalue()).decode("ascii"),
    })


# ---------- Engine: pymupdf4llm (born-digital, zero VRAM) ----------

# Below this digital-text length on a page, assume it's scanned and fall back
# to OCR. PyMuPDF returns the empty string for image-only PDFs.
_DIGITAL_TEXT_THRESHOLD = 80


def _has_digital_text(path, page):
    doc = pymupdf.open(path)
    try:
        if page < 0 or page >= len(doc):
            return False
        text = doc[page].get_text("text").strip()
        return len(text) >= _DIGITAL_TEXT_THRESHOLD
    finally:
        doc.close()


def ocr_digital(path, page):
    import pymupdf4llm

    send_stage("digital_extract")
    md = pymupdf4llm.to_markdown(
        path,
        pages=[page],
        show_progress=False,
        write_images=False,
    )
    if not isinstance(md, str):
        md = str(md)

    send_stage("saving")
    out_dir = _save_page_output(path, page, md, {})
    return {
        "type": "text",
        "engine": "pymupdf4llm",
        "page": page,
        "text": md,
        "saved_to": out_dir,
    }


# ---------- Engine: marker (optionally with LLM) ----------

def _marker_llm_config():
    return {
        "use_llm": True,
        "llm_service": "marker.services.openai.OpenAIService",
        "openai_base_url": os.environ.get(
            "MARKER_LLM_BASE_URL", "http://localhost:8080/v1"
        ),
        "openai_model": os.environ.get("MARKER_LLM_MODEL", "local"),
        "openai_api_key": os.environ.get("MARKER_LLM_API_KEY", "sk-no-key"),
    }


def ocr_marker(path, page, use_llm=False):
    from marker.converters.pdf import PdfConverter
    from marker.output import text_from_rendered
    from marker.providers.registry import provider_from_filepath
    from marker.builders.document import DocumentBuilder
    from marker.builders.line import LineBuilder
    from marker.builders.ocr import OcrBuilder
    from marker.builders.structure import StructureBuilder

    models = _ensure_marker()

    config = {"page_range": [page]}
    llm_service = None
    processor_list = None
    renderer = None
    if use_llm:
        from marker.config.parser import ConfigParser
        config.update(_marker_llm_config())
        cp = ConfigParser(config)
        config = cp.generate_config_dict()
        llm_service = cp.get_llm_service()
        processor_list = cp.get_processors()
        renderer = cp.get_renderer()

    send_stage("init_converter")
    converter = PdfConverter(
        artifact_dict=models,
        config=config,
        processor_list=processor_list,
        renderer=renderer,
        llm_service=llm_service,
    )

    send_stage("open_pdf")
    provider_cls = provider_from_filepath(path)
    provider = provider_cls(path, converter.config)

    send_stage("rasterize")
    doc_builder = DocumentBuilder(converter.config)
    document = doc_builder.build_document(provider)

    send_stage("layout")
    layout_builder = converter.resolve_dependencies(converter.layout_builder_class)
    layout_builder(document, provider)
    try:
        _emit_layout_overlay(document)
    except Exception:
        send({"type": "progress", "kind": "stage", "name": "overlay_failed"})

    send_stage("line_detection")
    line_builder = converter.resolve_dependencies(LineBuilder)
    line_builder(document, provider)

    if not doc_builder.disable_ocr:
        send_stage("ocr_recognition")
        ocr_builder = converter.resolve_dependencies(OcrBuilder)
        ocr_builder(document, provider)

    send_stage("structure")
    structure_builder = converter.resolve_dependencies(StructureBuilder)
    structure_builder(document)

    for processor in converter.processor_list:
        send_stage(f"processor:{type(processor).__name__}")
        processor(document)

    send_stage("render")
    renderer_inst = converter.resolve_dependencies(converter.renderer)
    rendered = renderer_inst(document)

    text, _, images = text_from_rendered(rendered)

    send_stage("saving")
    out_dir = _save_page_output(path, page, text, images)

    return {
        "type": "text",
        "engine": "marker_llm" if use_llm else "marker",
        "page": page,
        "text": text,
        "saved_to": out_dir,
    }


# ---------- Engine: VLM (Qwen2.5-VL-3B-Instruct-AWQ) ----------

VLM_PATH = os.environ.get(
    "OCR_VLM_PATH",
    os.path.join(REPOS_FOLDER, "Qwen2.5-VL-3B-Instruct-AWQ"),
)

VLM_PROMPT = (
    "Transcribe this document page exactly as it appears, "
    "formatted as GitHub-flavored Markdown. Preserve headings, "
    "lists, tables, code blocks, and emphasis. Render math in "
    "LaTeX. Output only the Markdown — no commentary."
)


def _ensure_vlm():
    global _vlm
    if _vlm is not None:
        return _vlm

    _unload_marker()
    send_stage("loading_vlm")

    # autoawq (deprecated, pinned to transformers<=4.51) imports
    # PytorchGELUTanh from transformers.activations at module load. The class
    # was removed in transformers 4.51+. Shim it back as nn.GELU(tanh) before
    # transformers' AWQ kernel triggers `import awq`.
    import transformers.activations as _act
    if not hasattr(_act, "PytorchGELUTanh"):
        import torch.nn as _nn
        import torch.nn.functional as _F

        class _PytorchGELUTanh(_nn.Module):
            def forward(self, x):
                return _F.gelu(x, approximate="tanh")

        _act.PytorchGELUTanh = _PytorchGELUTanh

    import torch
    from transformers import Qwen2_5_VLForConditionalGeneration, AutoProcessor

    if not os.path.isdir(VLM_PATH):
        raise FileNotFoundError(
            f"VLM model dir not found: {VLM_PATH}. "
            f"Set OCR_VLM_PATH to override."
        )

    model = Qwen2_5_VLForConditionalGeneration.from_pretrained(
        VLM_PATH,
        torch_dtype="auto",
        device_map="auto",
    )
    model.eval()

    processor = AutoProcessor.from_pretrained(
        VLM_PATH,
        min_pixels=256 * 28 * 28,
        max_pixels=1280 * 28 * 28,
    )

    _vlm = {"model": model, "processor": processor, "torch": torch}
    return _vlm


def ocr_vlm(path, page):
    from PIL import Image

    doc = pymupdf.open(path)
    try:
        if page < 0 or page >= len(doc):
            return {"type": "error", "message": f"page {page} out of range"}
        send_stage("rasterize")
        pix = doc[page].get_pixmap(dpi=200)
        img = Image.open(io.BytesIO(pix.tobytes("png"))).convert("RGB")
    finally:
        doc.close()

    vlm = _ensure_vlm()
    model = vlm["model"]
    processor = vlm["processor"]
    torch = vlm["torch"]

    messages = [{
        "role": "user",
        "content": [
            {"type": "image"},
            {"type": "text", "text": VLM_PROMPT},
        ],
    }]
    text_in = processor.apply_chat_template(
        messages, tokenize=False, add_generation_prompt=True
    )
    inputs = processor(
        text=[text_in],
        images=[img],
        padding=True,
        return_tensors="pt",
    ).to(model.device)

    send_stage("vlm_inference")
    with torch.inference_mode():
        out_ids = model.generate(
            **inputs,
            max_new_tokens=4096,
            do_sample=False,
        )
    trimmed = [o[len(i):] for i, o in zip(inputs.input_ids, out_ids)]
    md = processor.batch_decode(trimmed, skip_special_tokens=True)[0]

    send_stage("saving")
    out_dir = _save_page_output(path, page, md, {})

    return {
        "type": "text",
        "engine": "vlm",
        "page": page,
        "text": md,
        "saved_to": out_dir,
    }


# ---------- Engine: MinerU pipeline (layout + OCR + formula + table) ----------

# MinerU caches its own MineruPipelineModel via ModelSingleton (in-process),
# so we don't track a module-level handle here. We just unload Marker and VLM
# before invoking it to free VRAM, then let MinerU manage its own.

def ocr_mineru(path, page, lang="en"):
    import tempfile
    from pathlib import Path
    from mineru.cli.common import do_parse, read_fn

    _unload_marker()
    _unload_vlm()

    send_stage("loading_mineru")
    pdf_bytes = read_fn(Path(path))
    stem = _FS_UNSAFE.sub("_", os.path.splitext(os.path.basename(path))[0])

    with tempfile.TemporaryDirectory(prefix="mineru_") as tmpdir:
        send_stage("mineru_analyze")
        do_parse(
            output_dir=Path(tmpdir),
            pdf_file_names=[stem],
            pdf_bytes_list=[pdf_bytes],
            p_lang_list=[lang],
            backend="pipeline",
            parse_method="auto",
            start_page_id=page,
            end_page_id=page,
            formula_enable=True,
            table_enable=True,
        )
        md_candidates = list(Path(tmpdir).glob(f"{stem}/**/{stem}.md"))
        if not md_candidates:
            return {
                "type": "error",
                "message": "mineru produced no markdown output",
            }
        text = md_candidates[0].read_text(encoding="utf-8")

    send_stage("saving")
    out_dir = _save_page_output(path, page, text, {})
    return {
        "type": "text",
        "engine": "mineru",
        "page": page,
        "text": text,
        "saved_to": out_dir,
    }


# ---------- Quality gate for auto mode ----------

# Marker output below this many characters is treated as a recognition
# failure even before consulting the surya error classifier.
_MIN_USABLE_TEXT_CHARS = 20


def _vlm_available():
    return os.path.isdir(VLM_PATH)


def _marker_text_is_bad(text):
    """Use surya's OCRErrorPredictor to label marker output good/bad.

    Must be called while _marker_models is still loaded — ocr_error_model
    is part of that dict and will be freed when marker is unloaded.
    """
    cleaned = (text or "").strip()
    if len(cleaned) < _MIN_USABLE_TEXT_CHARS:
        return True
    global _marker_models
    if _marker_models is None:
        return False
    predictor = _marker_models.get("ocr_error_model")
    if predictor is None:
        return False
    try:
        result = predictor([cleaned])
    except Exception:
        return False
    labels = getattr(result, "labels", None) or []
    return bool(labels) and labels[0] == "bad"


# ---------- Engine dispatch ----------

def ocr(path, page, engine="auto", use_llm=False):
    if engine == "auto":
        if _has_digital_text(path, page):
            return ocr_digital(path, page)

        marker_result = ocr_marker(path, page, use_llm=use_llm)

        send_stage("ocr_quality_check")
        if _marker_text_is_bad(marker_result.get("text", "")):
            if _vlm_available():
                send_stage("marker_low_quality_falling_back_to_vlm")
                vlm_result = ocr_vlm(path, page)
                vlm_result["engine"] = "marker→vlm"
                vlm_result["fallback_reason"] = "ocr_error_predictor=bad"
                return vlm_result
            send_stage("marker_low_quality_but_vlm_unavailable")
            marker_result["quality_flag"] = "bad"
        return marker_result

    if engine == "digital":
        return ocr_digital(path, page)
    if engine == "marker":
        return ocr_marker(path, page, use_llm=use_llm)
    if engine == "marker_llm":
        return ocr_marker(path, page, use_llm=True)
    if engine == "vlm":
        return ocr_vlm(path, page)
    if engine == "mineru":
        return ocr_mineru(path, page)
    return {"type": "error", "message": f"unknown engine: {engine}"}


def _save_page_output(pdf_path, page, text, images):
    pdf_stem = _FS_UNSAFE.sub("_", os.path.splitext(os.path.basename(pdf_path))[0])
    out_dir = os.path.join(OUTPUT_ROOT, pdf_stem)
    os.makedirs(out_dir, exist_ok=True)

    md_path = os.path.join(out_dir, f"page_{page + 1:04d}.md")
    with open(md_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)

    for filename, img in (images or {}).items():
        if img.mode != "RGB":
            img = img.convert("RGB")
        img.save(os.path.join(out_dir, filename))

    return out_dir


def main():
    _start_metrics_thread_once()
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            cmd = json.loads(line)
        except json.JSONDecodeError as e:
            send({"type": "error", "message": f"bad json: {e}"})
            continue

        c = cmd.get("cmd")
        if c == "quit":
            return

        _metrics_active.set()
        try:
            if c == "render":
                result = render(
                    cmd["path"],
                    int(cmd.get("page", 0)),
                    int(cmd.get("dpi", 120)),
                )
            elif c == "ocr":
                result = ocr(
                    cmd["path"],
                    int(cmd.get("page", 0)),
                    engine=cmd.get("engine", "auto"),
                    use_llm=bool(cmd.get("use_llm", False)),
                )
            else:
                result = {"type": "error", "message": f"unknown command: {c}"}
        except Exception as e:
            result = {
                "type": "error",
                "message": str(e),
                "traceback": traceback.format_exc(),
            }
        finally:
            _metrics_active.clear()
        send(result)


if __name__ == "__main__":
    main()
