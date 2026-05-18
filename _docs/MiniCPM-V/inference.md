# Inference

The simplest local entry point is `chat.py`. It wraps several model eras
behind `MiniCPMVChat`.

`chat.py` chooses an implementation from the model path:

- paths containing `12B`: `OmniLMM12B`
- paths containing `MiniCPM-Llama3-V`: `MiniCPMV2_5`
- paths containing `MiniCPM-V-2_6`: `MiniCPMV2_6`
- otherwise: `MiniCPMV`

Common behavior:

- loads model and tokenizer with Hugging Face `AutoModel` or legacy OmniLMM
  classes
- accepts a base64-encoded image and a JSON conversation
- decodes the image with PIL
- calls the model's `chat` or `generate_vllm` path
- returns a text answer

MiniCPM-V 2.6 supports newer message content where each message can contain
text and image items. Its wrapper converts base64 image payloads into PIL
images before calling `model.chat`.

For larger models, the code includes optional multi-GPU loading through
`accelerate.load_checkpoint_and_dispatch`, `init_empty_weights`, and
manually adjusted device maps.

The root docs also show hosted API usage for MiniCPM-V 4.6 through a
Chat Completions style endpoint and note that MiniCPM-o 4.5 uses a realtime
API for full-duplex interaction.
