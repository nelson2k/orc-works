# Training

Training code is split into recognition training and CRAFT detection training.

Recognition trainer:

```text
trainer/
  train.py
  test.py
  dataset.py
  model.py
  modules/
  config_files/en_filtered_config.yaml
  trainer.ipynb
```

`trainer/README.md` is minimal and points users at `trainer.ipynb` plus YAML configs. The recognition trainer supports the familiar text-recognition pipeline pieces:

- transformation: e.g. `None`
- feature extraction: e.g. `VGG`
- sequence modeling: `BiLSTM`
- prediction: `CTC`

The sample `en_filtered_config.yaml` defines dataset paths, batch settings, model shape, optimizer settings, image size, and character sets.

CRAFT trainer:

```text
trainer/craft/
  trainSynth.py
  train.py
  train_distributed.py
  eval.py
  config/
  data/
  loss/
  metrics/
  model/
  utils/
```

`trainer/craft/README.md` documents:

- SynthText pretraining.
- custom / ICDAR-style data layout.
- single-GPU and multi-GPU commands.
- experiment outputs under `exp/[yaml]`.

Example CRAFT commands:

```text
CUDA_VISIBLE_DEVICES=0 python3 trainSynth.py --yaml=syn_train
CUDA_VISIBLE_DEVICES=0 python3 train.py --yaml=custom_data_train
CUDA_VISIBLE_DEVICES=0,1 python3 train_distributed.py --yaml=custom_data_train
```

