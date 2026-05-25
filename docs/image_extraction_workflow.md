# Result Image Extraction Workflow
This project includes a reusable image-analysis pipeline for extracting fracture-related information from screenshots (for example SPH workpiece fracture observations from long runs).

## Persistent requirements contract
The canonical extraction contract is stored at:

- `config/image_extraction_requirements.json`

This file defines:

- extraction defaults (thresholds, ROI preset, OCR default, output paths)
- required output fields expected by downstream analysis/prompt workflows
- maintenance rules for future updates

Update this contract first whenever extraction requirements change.

## CLI entrypoint
Use:

- `scripts/extract_results_image_info.py`

It supports single-image and batch-directory processing and produces:

- JSON summary (structured nested report)
- CSV summary (flat tabular report)
- optional debug artifacts (ROI crop, mask, annotated overlay)

### Python dependency setup
Install required package(s):

```bash
python -m pip install pillow pytesseract
```

Note:

- OCR text extraction requires a local Tesseract installation in addition to `pytesseract`.
- If Tesseract is missing, run with `--no-ocr`.

### Example (single image)
```bash
python scripts/extract_results_image_info.py \
  /path/to/image.png \
  --requirements-file config/image_extraction_requirements.json \
  --no-ocr \
  --pretty-json
```

### Example (directory batch)
```bash
python scripts/extract_results_image_info.py \
  results/screenshots \
  --requirements-file config/image_extraction_requirements.json \
  --recursive \
  --patterns "*.png,*.jpg" \
  --no-ocr
```

## Windows PowerShell wrapper
For consistent project usage on Windows:

- `scripts/run_image_extraction.ps1`

Example:

```powershell
.\scripts\run_image_extraction.ps1 `
  -Inputs "results\fracture_frames" `
  -Recursive `
  -NoOcr `
  -PrettyJson
```

The wrapper automatically points to `config/image_extraction_requirements.json` unless overridden.

## Output conventions
Default outputs are written under:

- `results/image_extract/`

Typical files:

- `image_extract_summary.json`
- `image_extract_summary.csv`
- `debug/*_overlay.png`
- `debug/*_mask.png`
- `debug/*_roi.png`

## Updating required fields safely
When changing `required_output_fields` in the requirements contract:

1) run the extractor on at least one representative image
2) keep strict requirement validation enabled
3) confirm the run exits successfully and no required fields are missing

This ensures future prompts can reuse the same extraction schema reliably.
