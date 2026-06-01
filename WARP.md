# Project Rules

## Simulation execution
- Run all simulation, validation, and preprocess production runs outside Warp.
- Execute simulation scripts/binaries in Windows PowerShell (`pwsh`/PowerShell) from the repository root.
- In Warp, only prepare/review commands and analyze outputs; do not launch simulation runs.
- Simulation run scripts and campaign execution tooling enforce this block in Warp by default.
- Use override only when intentional: `MFREE_ALLOW_WARP_SIM=1` (and `--allow-in-warp` for `campaign_mvp.py execute-manifest`).

## Repository context
- This is a standalone Windows-first fork of `iwf-inspire/mfree_iwf-ul-cut-refine`. Development happens on `GS76/mfree_iwf_ul_cut_refine_windows_ver` (origin) with `upstream` retained for selective pulls only; no merge-back to upstream is planned.
- Build and run from the repository root; the canonical executable path is `.\build\Release\mfree_iwf.exe`.

## Image extraction persistence
- Keep screenshot extraction requirements in `config/image_extraction_requirements.json` and update this contract before changing defaults or required fields.
- Reuse `scripts/extract_results_image_info.py` (and `scripts/run_image_extraction.ps1` on Windows) for future image-analysis prompts.
- Save extraction outputs under `results/image_extract/` so later prompts can reference prior artifacts consistently.

## PDF assessment persistence
- Reuse `scripts/assess_pdf.py` for PDF assessments and `scripts/run_pdf_assessment.ps1` for Windows PowerShell execution from repository root.
- Save PDF assessment artifacts under `results/pdf_assess/` (including `assessment_summary.json`, `assessment_preview.txt`, and extracted page text files).
- Keep keyword configuration in the script CLI (`--keywords`) so future PDF reviews remain consistent and reproducible.

## Version control
- Commit with descriptive, scoped messages using conventional prefixes (`docs:`, `fix:`, `feat:`, `chore:`, `refactor:`).
- Include `Co-Authored-By: Oz <oz-agent@warp.dev>` at the end of every commit message.
- Push to `origin master` after each completed change set.
- Use feature branches (`feature/*`, `fix/*`, `docs/*`) for non-trivial work; keep `master` deployable.

## Documentation maintenance
- Log all significant decisions, milestone completions, and repo-level changes in `docs/work_log.md`.
- When changing solver behavior, config schema, or CLI surface, update the corresponding documentation files:
  - `README.md` (overview, run modes, dependencies)
  - `CONFIG_SCHEMA.md` (JSON schema)
  - `CONFIGURATION_GUIDE.md` (user-facing parameter guide)
  - `BUILDING.md` (build instructions)
  - `README_WINDOWS.md` (Windows-specific notes)
  - `docs/coupling_thermal_mechanical.md` (thermal/mechanical coupling)
  - `docs/fe_tool_thermal_coupling.md` (FE tool coupling)
  - `TECHNICAL_OVERVIEW.md` (architecture changes)
- Keep command examples consistent: use `powershell` code fences and `.\build\Release\*.exe` paths.
