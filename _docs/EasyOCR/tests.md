# Tests

Unit-test harness files live under `unit_test/`.

Main files:

- `unit_test/README.md` - usage instructions.
- `unit_test/run_unit_test.py` - CLI wrapper around `UnitTest`.
- `unit_test/unit_test.py` - test harness implementation.
- `unit_test/data/EasyOcrUnitTestPackage.pickle` - packaged expected test data.
- `unit_test/make_test_solution.py` - creates/updates the expected package.
- `unit_test/demo.py` and `demo.ipynb` - interactive/demo runners.

Recommended command from the README:

```text
python ./unit_test/run_unit_test.py --easyocr ./easyocr --verbose 2 --test ./unit_test/EasyOcrUnitTestPackage.pickle --data_dir ./examples
```

The tests use example images under `examples/` and compare model behavior against stored expected outputs. Running them may require model weights to be present or downloadable, because `easyocr.Reader(...)` is instantiated during solution generation and tests.

