# Skills And Agent Assets

## Purpose

The `skills` directory contains agent-oriented skill definitions and helper scripts. These files package common GLM-OCR workflows so an agent can use the SDK consistently.

## Skill Directories

The folder contains these skill areas:

- `sdk`
- `glmocr`
- `glmocr-table`
- `glmocr-formula`
- `glmocr-handwriting`

## General SDK Skill

`skills/sdk/SKILL.md` appears to describe SDK-level setup or usage guidance.

## General GLM-OCR Skill

`skills/glmocr` contains:

- `SKILL.md`
- `scripts/glm_ocr_cli.py`
- `scripts/config_setup.py`
- `scripts/requirements.txt`
- `references/output_schema.md`

This is the broadest skill area. It includes a CLI helper script, config setup helper, requirements file, and output schema reference.

## Specialized Skills

Specialized OCR skills each contain their own `SKILL.md` and CLI helper script:

- `skills/glmocr-table`
- `skills/glmocr-formula`
- `skills/glmocr-handwriting`

These likely tune the same GLM-OCR workflow toward table extraction, formula extraction, and handwriting recognition.

## Helper Script Pattern

The repeated `glm_ocr_cli.py` scripts suggest the skills expose a command-oriented wrapper around GLM-OCR. The root SDK already has a CLI, but these skill scripts package usage in a way that is convenient for agent workflows.

## Output Schema Reference

`skills/glmocr/references/output_schema.md` documents the expected output schema for the general GLM-OCR skill.

The SDK/app output concepts include:

- Markdown text.
- JSON layout details.
- Page-level region lists.
- Labels such as text, table, formula, image.
- Bounding boxes.
- Optional cropped image references.

## Relationship To SDK

The skills sit outside the installable `glmocr` package. They are not part of the package discovery rules in root `pyproject.toml`, which include only `glmocr*`.

They are usage assets, not core runtime modules.

## Practical Takeaway

The skills folder is useful for understanding how the repository authors expect agents to call GLM-OCR:

- Minimal setup.
- CLI-style execution.
- Task-specific wrappers.
- Structured output interpretation.
