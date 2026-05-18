# Evaluation

The `eval_mm/` folder contains two evaluation paths.

VLM Eval Kit path:

- root: `eval_mm/vlmevalkit/`
- main script: `run.py`
- model adapters: `vlmeval/vlm/minicpm_v.py`
- shell launcher: `scripts/run_inference.sh`

The MiniCPM adapter file defines model wrappers for MiniCPM-V,
MiniCPM-Llama3-V 2.5, MiniCPM-V 2.6, and MiniCPM-o 2.6. It builds benchmark prompts,
chooses custom prompts or chain-of-thought behavior for some datasets, and
calls the model-specific generation method.

VQA evaluation path:

- root: `eval_mm/vqaeval/`
- main script: `eval.py`
- MiniCPM wrappers: `models/MiniCPM/minicpmv.py`
- shell launchers: `shell/run_inference.sh` and `shell/run_transform.sh`

The VQA path covers tasks such as TextVQA, DocVQA, and DocVQATest. The docs
describe expected dataset download folders and explain that DocVQATest output
can be transformed for official submission.

Some benchmark judging requires external LLM scoring. The evaluation docs
refer to `OPENAI_API_BASE` and `OPENAI_API_KEY` environment values for those
cases.
