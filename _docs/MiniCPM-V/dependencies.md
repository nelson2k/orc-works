# Dependencies

General Python requirements are in `requirements.txt`.

Core groups:

- `torch==2.1.2` and `torchvision==0.16.2`
- `transformers==4.40.0`
- `accelerate==0.30.1`
- `Pillow`, `numpy`, `opencv_python_headless`, `timm`, `einops`
- `gradio==4.41.0` and `modelscope_studio`
- evaluation and utility packages such as `jsonlines`, `sacrebleu`,
  `matplotlib`, `seaborn`, `nltk`, `openpyxl`, and `editdistance`
- video support through `decord`

MiniCPM-o 2.6 requirements are in `requirements_o2.6.txt`.

Additional groups:

- `torch==2.3.1`, `torchaudio==2.3.1`, `torchvision==0.18.1`
- `transformers==4.44.2`
- audio packages: `soundfile`, `librosa`, `vocos`, `vector-quantize-pytorch`
- video packages: `decord`, `moviepy`
- web API/demo packages: `fastapi`, `uvicorn`, `aiofiles`,
  `onnxruntime`, `gradio==4.44.1`, `pydantic==2.10.6`

The Vue web frontend under `web_demos/minicpm-o_2.6/web_server/` uses Node
and PNPM. Its main runtime dependencies include Vue 3, Vite, Element Plus,
Pinia, Vue Router, vue-i18n, axios, and browser VAD packages.
