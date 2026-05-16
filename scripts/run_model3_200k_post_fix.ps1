param(
    [string]$ResultsDir = "results\post_fix_200k_3GPa_w040_d10_fz03"
)

$ErrorActionPreference = "Stop"

# Wrapper for the agreed 200k Model-3 steady-state setup.
# Run from repo root in Windows PowerShell:
#   .\scripts\run_model3_200k_post_fix.ps1

& .\scripts\run_steady_state_100m_min.ps1 `
    -MaxSteps 200000 `
    -OutputFrames 40 `
    -ResultsDir $ResultsDir `
    -TensionCutoffPa 3000000000 `
    -RefineFrameWidthMm 0.40 `
    -RefineHaloLayers 0 `
    -RefineDepthFactor 1.0 `
    -InitialDepthFactor 0.3

if ($LASTEXITCODE -ne 0) {
    throw "200k Model 3 run failed (exit $LASTEXITCODE)"
}
