import sys
import json
import base64
import traceback

import pymupdf


_marker_models = None


def send(msg):
    print(json.dumps(msg), flush=True)


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
        from marker.models import create_model_dict
        _marker_models = create_model_dict()
    return _marker_models


def ocr(path, page):
    from marker.converters.pdf import PdfConverter
    from marker.output import text_from_rendered

    models = _ensure_marker()
    converter = PdfConverter(
        artifact_dict=models,
        config={"page_range": [page]},
    )
    rendered = converter(path)
    text, _, _ = text_from_rendered(rendered)
    return {
        "type": "text",
        "page": page,
        "text": text,
    }


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
