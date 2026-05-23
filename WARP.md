# Project Rules

## Simulation execution
- Run all simulation, validation, and preprocess production runs outside Warp.
- Execute simulation scripts/binaries in Windows PowerShell (`pwsh`/PowerShell) from the repository root.
- In Warp, only prepare/review commands and analyze outputs; do not launch simulation runs.

## Image extraction persistence
- Keep screenshot extraction requirements in `config/image_extraction_requirements.json` and update this contract before changing defaults or required fields.
- Reuse `scripts/extract_results_image_info.py` (and `scripts/run_image_extraction.ps1` on Windows) for future image-analysis prompts.
- Save extraction outputs under `results/image_extract/` so later prompts can reference prior artifacts consistently.

## PDF assessment persistence
- Reuse `scripts/assess_pdf.py` for PDF assessments and `scripts/run_pdf_assessment.ps1` for Windows PowerShell execution from repository root.
- Save PDF assessment artifacts under `results/pdf_assess/` (including `assessment_summary.json`, `assessment_preview.txt`, and extracted page text files).
- Keep keyword configuration in the script CLI (`--keywords`) so future PDF reviews remain consistent and reproducible.
