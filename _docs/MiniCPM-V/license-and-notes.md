# License And Notes

The repository includes an Apache 2.0 `LICENSE` file. The root README states
that MiniCPM-o/V model weights and code are open-sourced under Apache 2.0.

The README also includes a model statement: MiniCPM-o/V models generate
content from learned multimodal corpora, and generated outputs should not be
treated as representing the views or positions of the developers.

Practical caveats visible in the repo:

- many examples assume CUDA-capable GPUs
- some scripts use large model downloads from Hugging Face or ModelScope
- some evaluation tasks require manually downloaded datasets
- some benchmark judging depends on external LLM APIs
- MiniCPM-o real-time demos add audio, VAD, TTS, and WebSocket complexity
- older code paths and newer documentation coexist in the same repository
