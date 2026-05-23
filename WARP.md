# Project Rules

## Simulation execution
- Run all simulation, validation, and preprocess production runs outside Warp.
- Execute simulation scripts/binaries in Windows PowerShell (`pwsh`/PowerShell) from the repository root.
- In Warp, only prepare/review commands and analyze outputs; do not launch simulation runs.

## Image extraction persistence
- Keep screenshot extraction requirements in `config/image_extraction_requirements.json` and update this contract before changing defaults or required fields.
- Reuse `scripts/extract_results_image_info.py` (and `scripts/run_image_extraction.ps1` on Windows) for future image-analysis prompts.
- Save extraction outputs under `results/image_extract/` so later prompts can reference prior artifacts consistently.
