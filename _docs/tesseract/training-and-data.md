# Training And Data

Tesseract needs traineddata files at runtime. The repo includes config files under `tessdata/`, but not the full language model set. Runtime language files usually come from the upstream `tessdata`, `tessdata_fast`, or `tessdata_best` repositories.

Runtime lookup is controlled by:

- `--tessdata-dir PATH` on the CLI.
- `TESSDATA_PREFIX` in the environment.
- `datapath` passed to `TessBaseAPI::Init`.

Training tools are in `src/training` and documented in `doc/*.1.asc`.

Notable tools:

- `lstmtraining` - trains or fine-tunes LSTM networks from `.lstmf` samples and starter traineddata.
- `lstmeval` - evaluates an LSTM model.
- `text2image` - renders text with fonts to create training images.
- `unicharset_extractor` - creates unicharset data from box files.
- `combine_lang_model` - combines unicharset, script/language properties, words, punctuation, and numbers into a traineddata starter.
- `combine_tessdata` - lists, extracts, overwrites, unpacks, compacts, or combines `.traineddata` components.
- `wordlist2dawg` and `dawg2wordlist` - convert between word lists and DAWG dictionaries.

Traineddata components can include:

- `lang.lstm` - LSTM recognition model.
- `lang.lstm-unicharset`, recoder, punctuation/word/number DAWGs - LSTM language support.
- `lang.config` - language-specific parameter overrides.
- Legacy components such as `inttemp`, `pffmtable`, `normproto`, `unicharset`, and DAWGs when legacy engine support is used.

Upstream notes say training from scratch is not the normal user path. Fine-tuning an existing model, or replacing a layer, is the expected route for most customization.

