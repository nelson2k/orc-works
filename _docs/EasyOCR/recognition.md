# Recognition

Recognition inference lives in `easyocr/recognition.py`.

The packaged recognizers are CRNN-style models:

```text
image crop
  -> grayscale resize/pad to model height
  -> feature extractor
  -> two BiLSTM layers
  -> linear prediction
  -> CTC decode
```

Model definitions:

- `easyocr/model/model.py` - ResNet feature extractor plus BiLSTM and CTC head.
- `easyocr/model/vgg_model.py` - VGG feature extractor plus BiLSTM and CTC head.
- `easyocr/model/modules.py` - shared feature extractors and `BidirectionalLSTM`.

`Reader` chooses recognizers from `config.py`:

- generation 1 models: older, larger script models.
- generation 2 models: newer defaults for English, Latin, Chinese simplified, Japanese, Korean, Telugu, Kannada, Cyrillic, and others.
- `recog_network="standard"` auto-selects based on `lang_list`.
- explicit `recog_network` can force a model such as `latin_g1`.

Decoders:

- `greedy`
- `beamsearch`
- `wordbeamsearch`

Recognition options:

- `allowlist` restricts allowed output characters.
- `blocklist` suppresses specified characters.
- unsupported characters for the selected language are ignored by default.
- low-confidence crops can be retried with contrast adjustment.
- `rotation_info` tests rotated versions of each crop and keeps the highest-confidence result.
- Arabic model output is passed through `python-bidi` for right-to-left display.

Language support is resource-driven:

- `easyocr/character/*_char.txt` defines allowed characters per language.
- `easyocr/dict/*.txt` provides word lists for dictionary-aware decoding.

