# Tests

Tests live in `unittest/` and are GoogleTest-based.

Coverage areas include:

- API examples: `apiexample_test.cc`, `capiexample_test.cc`, `capiexample_c_test.c`, `baseapi_test.cc`.
- Threading and lifecycle: `baseapi_thread_test.cc`, `cleanapi_test.cc`.
- Layout/text order: `layout_test.cc`, `pagesegmode_test.cc`, `paragraphs_test.cc`, `tablefind_test.cc`, `tablerecog_test.cc`.
- LSTM/network code: `lstm_test.cc`, `lstm_recode_test.cc`, `lstm_squashed_test.cc`, `networkio_test.cc`, `recodebeam_test.cc`.
- Unicode and language data: `unicharset_test.cc`, `unichar_test.cc`, `validate_*_test.cc`.
- Low-level utilities: `matrix_test.cc`, `rect_test.cc`, `scanutils_test.cc`, `bitvector_test.cc`, `heap_test.cc`.

`unittest/README.md` describes extra fixture requirements. The test setup expects sibling directories such as `langdata_lstm`, `tessdata`, `tessdata_best`, and `tessdata_fast`, plus fonts and a googletest submodule.

Autotools test command from the README:

```text
make check
```

The tests are not self-contained in this checkout without downloading or providing the traineddata/font fixtures described above.

