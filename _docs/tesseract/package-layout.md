# Package Layout

Contents of `repos-folder/tesseract/`:

```text
include/tesseract/          public C and C++ headers
src/
  api/                      TessBaseAPI implementation and output renderers
  arch/                     SIMD detection and optimized dot/int matrix paths
  ccmain/                   high-level OCR control, page segmentation, iterators
  ccstruct/                 page, block, row, word, blob, geometry structures
  ccutil/                   parameters, unicode, serialization, tessdata manager
  classify/                 legacy classifier features and adaptive matching
  cutil/                    small legacy C utilities
  dict/                     dictionaries, DAWGs, hyphen/context logic
  lstm/                     LSTM network runtime and recognizer
  textord/                  text ordering, columns, tables, rows, word spacing
  training/                 command-line tools for traineddata and model training
  viewer/                   ScrollView debugging support
  wordrec/                  legacy word recognition and segmentation search
  tesseract.cpp             CLI program
doc/                        asciidoc manpages for CLI and training tools
tessdata/                   small configs and support files, not full models
unittest/                   GoogleTest-based unit tests and examples
test/                       test data / fixtures used by tests
cmake/                      CMake helpers and source lists
java/                       ScrollView Java UI pieces
nsis/                       Windows installer support
```

The clearest module inventory is `cmake/SourceLists.cmake`. It groups the library into `api`, `ccmain`, `ccstruct`, `ccutil`, `classify`, `dict`, `lstm`, `textord`, `viewer`, and `wordrec`.

The public API is intentionally thin. Most callers should only include headers from `include/tesseract`; internal headers under `src` expose implementation details and older engine machinery.

