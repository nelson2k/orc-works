# API

The main embedding surface is `tesseract::TessBaseAPI` in `include/tesseract/baseapi.h`.

Typical C++ flow:

```cpp
tesseract::TessBaseAPI api;
api.Init(datapath, "eng");
api.SetImage(pix);
char *text = api.GetUTF8Text();
// use text
delete[] text;
api.End();
```

Important methods:

- `Init(datapath, language, oem)` loads traineddata and initializes the engine.
- `SetVariable(name, value)` sets Tesseract parameters after initialization for non-init variables.
- `SetImage(...)` sets input image data, either raw bytes or a Leptonica `Pix`.
- `SetRectangle(...)` restricts recognition to a region.
- `Recognize(...)` runs OCR explicitly; text getters call it lazily if needed.
- `GetUTF8Text()`, `GetHOCRText(...)`, `GetTSVText(...)`, and iterator APIs expose results.
- `ProcessPage(...)` and `ProcessPages(...)` handle file/multipage workflows with renderers.

The C ABI in `include/tesseract/capi.h` mirrors the same lifecycle:

```text
TessBaseAPICreate
TessBaseAPIInit3
TessBaseAPISetImage2
TessBaseAPIGetUTF8Text
TessDeleteText
TessBaseAPIEnd
TessBaseAPIDelete
```

Output renderers live behind `TessResultRenderer` in `include/tesseract/renderer.h`. Concrete renderers include text, hOCR, ALTO, PAGE, TSV, PDF, UNLV, box, LSTM box, and word-string box renderers. Renderers can be chained with `insert`.

Threading note from `baseapi.h`: independent `TessBaseAPI` instances are mostly thread-safe, but some global parameters remain. Avoid changing classify/textord parameters with `SetVariable` across parallel instances unless that shared behavior is intended.

