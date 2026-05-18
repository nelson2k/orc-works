import sys
import json
import base64
import traceback

import pymupdf


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


def ocr(path, page, lang, dpi, tessdata):
    doc = pymupdf.open(path)
    try:
        pages = len(doc)
        if page < 0 or page >= pages:
            return {
                "type": "error",
                "message": f"page {page} out of range (doc has {pages} pages)",
            }
        p = doc[page]
        tp = p.get_textpage_ocr(language=lang, dpi=dpi, full=True, tessdata=tessdata)
        text = p.get_text("text", textpage=tp)
        return {
            "type": "text",
            "page": page,
            "pages": pages,
            "text": text,
        }
    finally:
        doc.close()


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
                    cmd.get("lang", "eng"),
                    int(cmd.get("dpi", 300)),
                    cmd.get("tessdata"),
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
