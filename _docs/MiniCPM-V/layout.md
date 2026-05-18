# Repository Layout

Top-level files:

- `README.md` and `README_zh.md`: project homepage and long-form model docs.
- `chat.py`: compact Python wrapper for local chat inference across older
  MiniCPM-V and OmniLMM variants.
- `requirements.txt`: general Python requirements for demos and evaluation.
- `requirements_o2.6.txt`: extra requirements for MiniCPM-o 2.6 audio/video
  demos.
- `LICENSE`: Apache 2.0 license text.

Main folders:

- `docs/`: model-specific docs, API guide, best practices, framework notes,
  inference notes, and older model pages.
- `web_demos/`: Gradio demos for MiniCPM-V variants and a larger MiniCPM-o
  2.6 real-time web demo.
- `finetune/`: supervised finetuning scripts, dataset processing, trainer
  customization, DeepSpeed configs, and shell launchers.
- `eval_mm/`: evaluation integrations, including a VLM Eval Kit subtree and
  VQA evaluation scripts.
- `omnilmm/`: legacy OmniLMM model, conversation, image transform, resampler,
  and preprocessing code.
- `assets/`: images, videos, audio samples, diagrams, benchmark screenshots,
  demo media, and UI assets.
