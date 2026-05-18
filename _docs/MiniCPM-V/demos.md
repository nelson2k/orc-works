# Web Demos

The `web_demos/` folder contains multiple demo styles.

Gradio demos:

- `web_demo.py`
- `web_demo_2.5.py`
- `web_demo_2.6.py`
- `web_demo_streamlit.py`
- `web_demo_streamlit-2_5.py`
- `web_demo_streamlit-minicpmv2_6.py`

`web_demo_2.6.py` is a Gradio demo for `openbmb/MiniCPM-V-2_6`. It loads
the model with `transformers`, accepts image and video uploads, samples video
frames through `decord`, builds multimodal messages, calls `model.chat`, and
launches on port `8885`.

MiniCPM-o 2.6 real-time demo:

- `web_demos/minicpm-o_2.6/model_server.py`: FastAPI backend.
- `web_demos/minicpm-o_2.6/chatbot_web_demo_o2.6.py`: Python demo UI entry.
- `web_demos/minicpm-o_2.6/vad_utils.py`: voice activity detection helpers.
- `web_demos/minicpm-o_2.6/web_server/`: Vue 3 frontend.

The MiniCPM-o backend loads `openbmb/MiniCPM-o-2_6` by default, initializes
TTS, handles audio/image streaming state, and exposes HTTP, SSE, WebSocket,
health, stop, feedback, and option endpoints.

The Vue frontend uses Vite, Element Plus, Pinia, Vue Router, vue-i18n,
axios, and browser VAD utilities. Its package scripts are `dev`, `build`,
`preview`, `lint`, and `format`.
