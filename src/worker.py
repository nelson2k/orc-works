import sys
import os
import io
import json
import base64
import threading
import traceback


_send_lock = threading.Lock()


def send(msg):
    line = json.dumps(msg) + "\n"
    with _send_lock:
        sys.stdout.write(line)
        sys.stdout.flush()


def send_stage(name):
    send({"type": "progress", "kind": "stage", "name": name})


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


_marker_models = None


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


def ocr(path, page):
    from marker.converters.pdf import PdfConverter
    from marker.output import text_from_rendered
    from marker.providers.registry import provider_from_filepath
    from marker.builders.document import DocumentBuilder
    from marker.builders.line import LineBuilder
    from marker.builders.ocr import OcrBuilder
    from marker.builders.structure import StructureBuilder

    models = _ensure_marker()
    config = {"page_range": [page]}

    send_stage("init_converter")
    converter = PdfConverter(artifact_dict=models, config=config)

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
    renderer = converter.resolve_dependencies(converter.renderer)
    rendered = renderer(document)

    text, _, _ = text_from_rendered(rendered)
    return {"type": "text", "page": page, "text": text}


def main():
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
        try:
            if c == "quit":
                return
            elif c == "render":
                send(render(
                    cmd["path"],
                    int(cmd.get("page", 0)),
                    int(cmd.get("dpi", 120)),
                ))
            elif c == "ocr":
                send(ocr(
                    cmd["path"],
                    int(cmd.get("page", 0)),
                ))
            else:
                send({"type": "error", "message": f"unknown command: {c}"})
        except Exception as e:
            send({
                "type": "error",
                "message": str(e),
                "traceback": traceback.format_exc(),
            })


if __name__ == "__main__":
    main()
