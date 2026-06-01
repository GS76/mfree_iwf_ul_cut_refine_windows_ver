# Campaign MVP workflow
This document describes the new MVP campaign tooling for structured DOE execution and normalized result aggregation.
## Scope (MVP gate)
MVP covers: campaign spec validation, initial deterministic design generation, manifest-driven execution adapter, and canonical normalized dataset export with quality flags.
Surrogate-guided sequential recommendation is intentionally out of MVP scope.
## Entry point
Use `scripts/campaign_mvp.py` with subcommands:
- `validate-spec`
- `generate-manifest`
- `execute-manifest`
- `normalize-results`
## External execution enforcement
Non-dry simulation execution is blocked when launched from Warp sessions.
- Run production simulation commands from external Windows PowerShell (`pwsh`/PowerShell) at repo root.
- Allowed in Warp by explicit override only:
  - CLI flag: `--allow-in-warp` (for `execute-manifest` only)
  - Environment variable: `MFREE_ALLOW_WARP_SIM=1`
- Guarded PowerShell run scripts in `scripts/` enforce the same behavior.
## Example (Model 5 MVP spec)
1) Validate spec
```powershell
python scripts/campaign_mvp.py validate-spec --spec config/campaign_model5_mvp.json
```
2) Generate initial design manifest
```powershell
python scripts/campaign_mvp.py generate-manifest --spec config/campaign_model5_mvp.json
```
3) Dry-run adapter commands (no simulation execution)
```powershell
python scripts/campaign_mvp.py execute-manifest --manifest results/campaigns/model5_mvp_phase1/artifacts/manifest.json --dry-run
```
3b) Execute manifest (external PowerShell session required by default)
```powershell
python scripts/campaign_mvp.py execute-manifest --manifest results/campaigns/model5_mvp_phase1/artifacts/manifest.json
```
4) Build canonical normalized dataset
```powershell
python scripts/campaign_mvp.py normalize-results --manifest results/campaigns/model5_mvp_phase1/artifacts/manifest.json
```
## Canonical output fields
The normalized dataset includes at minimum:
- `campaign_id`, `run_id`, `design_iteration`, `parameter_set`
- `script_entrypoint`, `results_dir`, `status`, `exit_code`
- `started_at`, `ended_at`, `stdout_log`, `stderr_log`
- objective columns (`objective_*`)
- constraint columns (`constraint_*_value`, `constraint_*_satisfied`)
- `quality_flags`
## Artifact locations
All campaign artifacts are written under:
`results/campaigns/<campaign_id>/`
- `artifacts/validated_spec.json`
- `artifacts/manifest.json`
- `artifacts/logs/*.stdout.log`, `artifacts/logs/*.stderr.log`
- `artifacts/normalized_results.csv`
- `runs/<run_id>/` for run-specific results directories
## Archiving a completed campaign snapshot
After verification, archive the campaign directory into a timestamped baseline snapshot while preserving the live campaign workspace:
```powershell
$src = "results/campaigns/model5_mvp_phase1"
$stamp = Get-Date -Format "yyyyMMdd-HHmm"
$dst = "results/baseline/$stamp/model5_mvp_phase1_campaign_success"
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
Copy-Item -Path $src -Destination $dst -Recurse -Force
```
Record the final archive path and key metrics in `docs/work_log.md`.
