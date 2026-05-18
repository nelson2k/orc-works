# Finetuning

The `finetune/` folder provides official scripts for supervised finetuning
of supported MiniCPM-V and MiniCPM-o models.

Supported model families in the finetune readme include:

- MiniCPM-V 4.0
- MiniCPM-o 2.6
- MiniCPM-V 2.6
- MiniCPM-Llama3-V 2.5
- MiniCPM-V 2.0

Data format:

- JSON list of samples
- each sample has an `id`, an image path or image dictionary, and
  `conversations`
- single-image samples use `<image>`
- multi-image samples use placeholders like `<image_00>`, `<image_01>`
- if no placeholder is present, the image is inserted at the front

Training entry points:

- `finetune.py`: parses model/data/training/LoRA args, loads model and
  tokenizer, prepares datasets, configures LoRA when requested, then runs
  `CPMTrainer`.
- `finetune_ds.sh`: full or partial parameter training through `torchrun`
  and DeepSpeed Zero-3.
- `finetune_lora.sh`: LoRA training through `torchrun`, PEFT, and DeepSpeed
  Zero-2.

Important knobs:

- `llm_type`: `minicpm`, `llama3`, or `qwen`
- `tune_vision`: whether to train the visual module
- `tune_llm`: whether to train the language model
- `use_lora`: enable PEFT LoRA
- `model_max_length`: defaults to 2048; multi-image SFT commonly needs 4096
- `max_slice_nums`: controls image slicing and token cost

The dataset code builds token IDs, labels, image bounds, pixel values, target
sizes, position IDs, and attention masks for multimodal supervised training.
