# check_timestep_100m_min.ps1
#
# Purpose
# -------
# Preprocess-only run at 100 m/min with nbox=61 for Model 3
# (dynamic refinement).  Prints the full coupled-timestep-estimator
# breakdown so the velocity-adaptive dt_empirical can be inspected
# and compared with the legacy hard-coded 5e-10 s value.
#
# Expected output (model 3, nbox=61, 100 m/min):
#   nbox=61 (particle layers through thickness, dx ~ 8.3 um)
#   timestep estimate: dt=<value> limiter=empirical
#     wp_mech=...  wp_therm=...  empirical=<0.20*hdx*dx/(c0+vc)>
#
# At 100 m/min  vc =  1.67 m/s  ->  dt_empirical ~ 4.678e-10 s
# At 500 m/min  vc =  8.33 m/s  ->  dt_empirical ~ 4.672e-10 s
# (difference < 0.2 % because c0 >> vc; confirms formula is correct)
#
# Usage (from repo root):
#   .\scripts\check_timestep_100m_min.ps1
#
param(
	[string]$Exe  = ".\build\Release\mfree_iwf.exe",
	[string]$Mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Exe))
{ throw "Executable not found: $Exe" 
}
if (-not (Test-Path $Mesh))
{ throw "FE tool mesh not found: $Mesh" 
}

# ── geometry / physics ────────────────────────────────────────────────────────
$env:MFREE_CUTTING_SPEED_M_MIN    = "100"
$env:MFREE_NBOX                   = "61"
$env:MFREE_FEED_PER_REV_MM        = "0.2"
$env:MFREE_BASE_TARGET_FEED_MM    = "0.1"
$env:MFREE_WORKPIECE_THICKNESS_MM = "0.5"

# ── FE tool ───────────────────────────────────────────────────────────────────
$env:MFREE_FE_TOOL_MSH             = $Mesh
$env:MFREE_NO_RIGID_TOOL           = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_MU              = "0.35"

# WC-Co tool material (Sima & Ozel 2010 / standard WC-Co)
$env:MFREE_FE_TOOL_RHO             = "14500"   # kg/m^3
$env:MFREE_FE_TOOL_CP              = "200"     # J/kg*K
$env:MFREE_FE_TOOL_K               = "80"      # W/m*K
$env:MFREE_FE_TOOL_E               = "6e11"    # Pa
$env:MFREE_FE_TOOL_NU              = "0.22"
$env:MFREE_FE_TOOL_ALPHA           = "4.5e-6"  # 1/K
$env:MFREE_FE_TOOL_FIX_TAGS        = "114"

# ── timestep estimator ────────────────────────────────────────────────────────
# Print the full breakdown: all individual CFL limits + the winning limiter.
$env:MFREE_TIMESTEP_PRINT             = "1"
$env:MFREE_TIMESTEP_WP_MECH_SAFETY    = "0.25"
$env:MFREE_TIMESTEP_WP_THERM_SAFETY   = "0.20"
$env:MFREE_TIMESTEP_TOOL_MECH_SAFETY  = "0.90"
$env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "1.00"
$env:MFREE_TIMESTEP_INTERFACE_SAFETY  = "0.50"

# ── preprocess only ───────────────────────────────────────────────────────────
$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_CLEAN_RESULTS   = "1"
$env:MFREE_RESULTS_DIR     = "results\check_timestep_100m_min"

Write-Host ""
Write-Host "=========================================================="
Write-Host " Timestep check: 100 m/min, nbox=61, Model 3"
Write-Host " Inspect 'dt=' and 'limiter=' lines below."
Write-Host "=========================================================="
Write-Host ""

& $Exe -m 3
if ($LASTEXITCODE -ne 0)
{ throw "mfree_iwf.exe exited with code $LASTEXITCODE" 
}

Write-Host ""
Write-Host "Done.  Key values to verify:"
Write-Host "  dt_empirical = 0.20 * hdx * dx / (c0 + vc)"
Write-Host "  At 100 m/min (vc=1.67 m/s): expected ~4.678e-10 s"
Write-Host "  At 500 m/min (vc=8.33 m/s): expected ~4.672e-10 s  (< 0.2 % difference)"
Write-Host "  limiter should read: empirical"
Write-Host "  wp_mech should read: ~5.85e-10 s  (looser -> empirical cap is binding)"

$global:LASTEXITCODE = 0
