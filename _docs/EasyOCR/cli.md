# CLI

The console script is registered in `setup.py`:

```text
easyocr = easyocr.cli:main
```

Basic use:

```text
easyocr -l en -f image.jpg
easyocr -l ch_sim en -f chinese.jpg --detail=1 --gpu=True
```

The CLI is a thin wrapper around `Reader(...).readtext(...)`.

Important arguments:

- `-l`, `--lang` - one or more language codes, required.
- `-f`, `--file` - input image path, required.
- `--gpu` - enable GPU selection.
- `--model_storage_directory` - directory for downloaded model weights.
- `--user_network_directory` - directory for custom recognizer Python/YAML files.
- `--recog_network` - built-in or custom recognizer name.
- `--download_enabled` - allow or prevent model downloads.
- `--detector`, `--recognizer` - initialize either or both modules.
- `--decoder` - `greedy`, `beamsearch`, or `wordbeamsearch`.
- `--allowlist`, `--blocklist` - character filtering.
- `--detail` - `0` for text-only output, `1` for full output.
- `--paragraph` - merge boxes into paragraphs.
- `--output_format` - `standard`, `dict`, or `json`.

Detection and grouping thresholds exposed by the CLI match the `readtext` method: `min_size`, `contrast_ths`, `text_threshold`, `low_text`, `link_threshold`, `canvas_size`, `mag_ratio`, `slope_ths`, `ycenter_ths`, `height_ths`, `width_ths`, `y_ths`, `x_ths`, and `add_margin`.

