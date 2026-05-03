# run_steady_state_100m_min.ps1
#
# Purpose
# -------
# Production run at 100 m/min with nbox=61 (dx ~ 8.3 um), Model 3
# (dynamic refinement), long enough to reach thermal quasi-steady
# state at the tool-workpiece interface.
#
# Step count rationale (model 3, nbox=61, 100 m/min):
#   dt_empirical = 0.20 * hdx * dx / (c0 + vc)
#              ~ 4.678e-10 s
#
#   500 000 steps  ->  t_total ~ 234 us  ->  cut distance ~ 0.39 mm
#
#   Thermal quasi-steady state at the contact zone typically develops
#   after ~0.1-0.3 mm of cut (100-180 us).  500 000 steps is therefore
#   comfortably past that threshold and within one full 1 mm cut.
#
#   Estimated wall-clock time: ~60-90 min on a 12-thread workstation.
#
# Outputs (in $ResultsDir):
#   cutting_energy.csv   -- per-step energy partition incl.
#                           step_contact_E_tool_frac / cum_contact_E_tool_frac
#   cutting_thermal.csv  -- per-step P_cond, P_fric, tool T range
#   cutting_metrics.csv  -- per-step workpiece mechanics diagnostics
#   out_NNNNNN.vtk       -- workpiece particle snapshots (100 frames)
#   fe_tool_NNNNNN.vtk   -- FE tool snapshots (100 frames)
#
# Usage (from repo root):
#   .\scripts\run_steady_state_100m_min.ps1
#   .\scripts\run_steady_state_100m_min.ps1 -MaxSteps 10000   # quick smoke test
#
param(
	[string]$Exe          = ".\build\Release\mfree_iwf.exe",
	[string]$Mesh                = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh",
	[string]$ResultsDir          = "results\steady_state_100m_min_Full",
	[int]   $MaxSteps            = 500000,  # steps to run
	[int]   $OutputFrames        = 100,     # VTK snapshots + energy-CSV rows
	[double]$RefineDepthFactor   = 2.0,     # moving-frame depth = factor * target cut/feed depth
	[double]$RefineFrameWidthMm  = 0.50,    # moving-frame width ahead of tool, mm
	[int]   $RefineHaloLayers    = 2,       # extra coarse-grid layers around the moving frame
	[double]$TensionCutoffPa     = 3e9,     # EOS tension cutoff (Pa); 3 GPa default, try 1e9 for tighter clamp
	[double]$DensityFloorFrac    = 0.001    # density floor as fraction of rho0 (0 = disabled)
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Exe))
{ throw "Executable not found: $Exe"
}
if (-not (Test-Path $Mesh))
{ throw "FE tool mesh not found: $Mesh"
}

# ── output directory ──────────────────────────────────────────────────────────
New-Item -ItemType Directory -Force -Path $ResultsDir | Out-Null
$env:MFREE_RESULTS_DIR   = $ResultsDir
$env:MFREE_CLEAN_RESULTS = "1"

# ── geometry / physics ────────────────────────────────────────────────────────
$env:MFREE_CUTTING_SPEED_M_MIN    = "100"   # m/min  (1.67 m/s)
$env:MFREE_NBOX                   = "61"    # particle layers through thickness
$env:MFREE_FEED_PER_REV_MM        = "0.2"
$env:MFREE_BASE_TARGET_FEED_MM    = "0.1"
$env:MFREE_WORKPIECE_THICKNESS_MM = "0.5"

# ── dynamic refinement frame ──────────────────────────────────────────────────
$env:MFREE_REFINE_DEPTH_FACTOR    = "$RefineDepthFactor"
$env:MFREE_REFINE_FRAME_WIDTH_MM  = "$RefineFrameWidthMm"
$env:MFREE_REFINE_HALO_LAYERS     = "$RefineHaloLayers"

# ── FE tool ───────────────────────────────────────────────────────────────────
$env:MFREE_FE_TOOL_MSH             = $Mesh
$env:MFREE_NO_RIGID_TOOL           = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_MU              = "0.35"

# WC-Co tool material (Sima & Ozel 2010 / standard WC-Co)
$env:MFREE_FE_TOOL_RHO             = "14500"   # kg/m^3
$env:MFREE_FE_TOOL_CP              = "200"     # J/kg*K  (correct WC-Co value)
$env:MFREE_FE_TOOL_K               = "80"      # W/m*K
$env:MFREE_FE_TOOL_E               = "6e11"    # Pa
$env:MFREE_FE_TOOL_NU              = "0.22"
$env:MFREE_FE_TOOL_ALPHA           = "4.5e-6"  # 1/K
$env:MFREE_FE_TOOL_FIX_TAGS        = "114"     # rear-face Dirichlet

# Explicit FE tool time integration inside the SPH contact loop
$env:MFREE_DEFORMABLE_FE_TOOL                    = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT           = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "100"
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS -ErrorAction SilentlyContinue
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS  -ErrorAction SilentlyContinue
$env:MFREE_FE_TOOL_RAYLEIGH_A0     = "0"
$env:MFREE_FE_TOOL_RAYLEIGH_A1     = "0"

# ── thermal contact ───────────────────────────────────────────────────────────
# h_full/h_sep now default to 1e6/1e4 W/m^2*K (updated in contact.cpp).
# Explicit values set here for full reproducibility and easy DOE override.
$env:MFREE_THERMAL_H_FULL          = "1000000"  # W/m^2*K  full-pressure contact
$env:MFREE_THERMAL_H_SEP           = "10000"    # W/m^2*K  partial / air-gap contact
$env:MFREE_THERMAL_P_REF           = "1e9"      # Pa       reference pressure

# Friction heat partition (workpiece / tool).  Defaults (0.8/0.2) set
# explicitly so DOE variants are unambiguous.
$env:MFREE_THERMAL_FRAC_WP         = "0.8"
$env:MFREE_THERMAL_FRAC_TOOL       = "0.2"

# Per-step dT limiter raised to 5 K (default 1 K) so the limiter does not
# suppress interface exchange as the contact-zone temperature rises toward
# steady state.  Monitor cum_suppression_ratio in cutting_energy.csv;
# reduce to 1 K if it exceeds 5 %.
$env:MFREE_THERMAL_MAX_DT_PER_STEP = "5"

# ── timestep estimator ────────────────────────────────────────────────────────
# Print full CFL breakdown at startup so dt can be confirmed in the log.
$env:MFREE_TIMESTEP_PRINT             = "1"
$env:MFREE_TIMESTEP_WP_MECH_SAFETY    = "0.25"
$env:MFREE_TIMESTEP_WP_THERM_SAFETY   = "0.20"
$env:MFREE_TIMESTEP_TOOL_MECH_SAFETY  = "0.90"
$env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "1.00"
$env:MFREE_TIMESTEP_INTERFACE_SAFETY  = "0.50"

# T_FINAL_SCALE = 1.0: the full 1-mm-cut physics window is available.
# The run stops at MaxSteps (well before t_final) so no adjustment needed.
$env:MFREE_T_FINAL_SCALE = "1.0"

# ── tensile instability control ──────────────────────────────────────────────
# Tension cutoff (Pa): caps the maximum hydrostatic tensile pressure in the
# EOS so that free-surface / chip particles cannot develop runaway pressures
# of tens of GPa.  Default 3 GPa; pass -TensionCutoffPa 1e9 for a tighter
# clamp closer to the physical spall/fracture strength of Ti-6Al-4V.
$env:MFREE_TENSION_CUTOFF     = "$TensionCutoffPa"
# Density floor (fraction of rho0): belt-and-suspenders guard so that
# rho never goes negative, which would corrupt the 1/rho² stress-divergence
# terms.  0.001 -> rho_min = 4.43 kg/m³ for Ti-6Al-4V (0.1% of reference).
$env:MFREE_DENSITY_FLOOR_FRAC = "$DensityFloorFrac"

# ── run control ───────────────────────────────────────────────────────────────
$env:MFREE_MAX_STEPS = "$MaxSteps"

# Output frequency: energy CSV + VTK written together at fixed intervals.
# MFREE_LOG_TIME_STEP_DATA_EVERY_STEP=0 prevents a 500 000-row energy CSV.
$outputFreq = [Math]::Max(1, [int]($MaxSteps / $OutputFrames))
$env:MFREE_OUTPUT_FREQ                   = "$outputFreq"
$env:MFREE_LOG_TIME_STEP_DATA_EVERY_STEP = "0"   # energy CSV at output_freq only

# ── logging ───────────────────────────────────────────────────────────────────
$env:MFREE_LOG_THERMAL       = "1"   # cutting_thermal.csv
$env:MFREE_LOG_METRICS       = "1"   # cutting_metrics.csv
$env:MFREE_LOG_ENERGY        = "1"   # cutting_energy.csv
$env:MFREE_LOG_VTK_WORKPIECE = "1"   # out_NNNNNN.vtk
$env:MFREE_LOG_VTK_FE_TOOL   = "1"   # fe_tool_NNNNNN.vtk
$env:MFREE_LOG_VTK_TOOL      = "0"   # rigid tool VTK not used

# ── startup summary ───────────────────────────────────────────────────────────
$dt_approx  = 4.678e-10
$t_total_us = [Math]::Round($MaxSteps * $dt_approx * 1e6, 1)
$cut_mm     = [Math]::Round($MaxSteps * $dt_approx * (100.0 / 60.0) * 1000, 3)

Write-Host ""
Write-Host "============================================================"
Write-Host " Steady-state run: 100 m/min, nbox=61, Model 3"
Write-Host "   MaxSteps     = $MaxSteps"
Write-Host "   dt (approx)  = $dt_approx s"
Write-Host "   t_total      ~ $t_total_us us"
Write-Host "   cut distance ~ $cut_mm mm"
Write-Host "   VTK frames   = $OutputFrames  (every $outputFreq steps)"
Write-Host "   Refinement   = depth_factor $RefineDepthFactor, width ${RefineFrameWidthMm} mm, halo $RefineHaloLayers layers"
Write-Host "   TensionCutoff= $TensionCutoffPa Pa  DensityFloor= $DensityFloorFrac * rho0"
Write-Host "   Results      -> $ResultsDir"
Write-Host "============================================================"
Write-Host ""

# ── preprocess pass ───────────────────────────────────────────────────────────
Write-Host "--- Preprocess / geometry check (Model 3) ---"
$env:MFREE_PREPROCESS_ONLY = "1"
& $Exe -m 3
if ($LASTEXITCODE -ne 0)
{ throw "Preprocess failed (exit $LASTEXITCODE)"
}

# ── production run ────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "--- Starting $MaxSteps-step production run (Model 3) ---"
$env:MFREE_PREPROCESS_ONLY = "0"
# Preprocess already cleaned and populated the results dir (precheck report,
# validation summary, initial VTK).  Do NOT clean again or those outputs are lost.
$env:MFREE_CLEAN_RESULTS   = "0"
$startTime = Get-Date
& $Exe -m 3
$elapsed = (Get-Date) - $startTime
if ($LASTEXITCODE -ne 0)
{ throw "Production run failed (exit $LASTEXITCODE)"
}

Write-Host ""
Write-Host "============================================================"
Write-Host (" Run complete in {0:hh\:mm\:ss}" -f $elapsed)
Write-Host " Key files to inspect:"
Write-Host "   $ResultsDir\cutting_energy.csv"
Write-Host "     -> cum_contact_E_tool_frac  (target >= 0.10 at steady state)"
Write-Host "     -> cum_suppression_ratio    (target <  0.05)"
Write-Host "     -> closure_residual_pct     (target <  1 %)"
Write-Host "   $ResultsDir\cutting_thermal.csv"
Write-Host "     -> tool_Tmax, wp_Tavg, P_cond_W, P_fric_W"
Write-Host "============================================================"

$global:LASTEXITCODE = 0
