# tesseract - notes

Source: `repos-folder/tesseract`. Version in this checkout: `5.5.2`. License: Apache-2.0.

Tesseract is a C++ OCR engine with two public faces:

- `libtesseract`, used through the C++ API in `include/tesseract/baseapi.h` or the C API in `include/tesseract/capi.h`.
- `tesseract`, the command-line OCR program in `src/tesseract.cpp`.

The engine reads images through Leptonica, loads language/script models from `tessdata`, and emits plain text or structured OCR formats such as hOCR, PDF, TSV, ALTO, and PAGE XML.

The modern recognition path is LSTM-based. The repository still contains the older Tesseract 3-style classifier/word-recognition code and can build with the legacy engine unless `DISABLED_LEGACY_ENGINE` is enabled.

Good first files:

- `README.md` - upstream overview.
- `src/tesseract.cpp` - command-line entry point and option handling.
- `include/tesseract/baseapi.h` - main C++ embedding API.
- `include/tesseract/capi.h` - C ABI wrapper.
- `cmake/SourceLists.cmake` - compact map of library modules.
- `doc/tesseract.1.asc` - local manpage source for CLI behavior.

