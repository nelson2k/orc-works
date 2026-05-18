# Legacy OmniLMM Code

The `omnilmm/` package contains older OmniLMM implementation pieces used by
`chat.py` when the model path contains `12B`.

Main parts:

- `omnilmm/model/omnilmm.py`: model definition based on Mistral classes.
- `omnilmm/model/resampler.py`: learned query resampler and positional
  embedding helpers.
- `omnilmm/model/utils.py`: image transforms, random augmentation, stopping
  criteria, base64 helpers, and distributed helpers.
- `omnilmm/conversation.py`: conversation history and prompt formatting.
- `omnilmm/train/train_utils.py`: OmniLMM preprocessing for generation and
  training.
- `omnilmm/utils.py`: logger helpers, disabled torch initialization, and
  moderation stub.

Architecture notes from the code:

- the language backbone is Mistral-style
- the vision tower is an EVA02 CLIP-style timm model
- a `Resampler` compresses vision features into a fixed number of query
  tokens
- image embeddings are inserted between `<im_start>` and `<im_end>` tokens
- `generate_vllm` wraps generation with image inputs

This code is legacy support for older OmniLMM checkpoints, not the main path
for the newest MiniCPM-V 4.x model docs.
