# run_model5_fe_tool.ps1
#
# Purpose
# -------
# Model 5: SPH workpiece (dynamic refinement) with fully coupled deformable FE tool.
# Runs a long-duration cutting simulation at 100 m/min with thermal and mechanical
# FE tool coupling to reach thermal quasi-steady state at the tool-workpiece interface.
#
# Step count rationale (model 5, nbox=61, 100 m/min):
#   dt_empirical = 0.20 * hdx * dx / (c0 + vc)
#              ~ 4.678e-10 s
#
#   500 000 steps  ->  t_total ~ 234 us  ->  cut distance ~ 0.39 mm
#
# Outputs (in $ResultsDir):
#   cutting_energy.csv   -- per-step energy partition incl.
#                           cum_contact_E_tool_frac
#   cutting_thermal.csv  -- per-step P_cond, P_fric, tool T range
#   cutting_metrics.csv  -- per-step workpiece mechanics diagnostics
#   out_NNNNNN.vtk       -- workpiece particle snapshots
#   fe_tool_NNNNNN.vtk   -- FE tool snapshots
#
# Usage (from repo root):
#   .\scripts\run_model5_fe_tool.ps1
#   .\scripts\run_model5_fe_tool.ps1 -MaxSteps 10000   # quick smoke test
#
param(
	[string]$Exe          = ".\build\Release\mfree_iwf.exe",
	[string]$Mesh                = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh",
	[string]$ResultsDir          = "",
	[int]   $MaxSteps            = 500000,  # steps to run
	[int]   $OutputFrames        = 100,     # VTK snapshots + CSV rows
	[double]$RefineDepthFactor   = 2.0,     # moving-frame depth = factor * target cut/feed depth
	[double]$RefineFrameWidthMm  = 0.50,    # moving-frame width ahead of tool, mm
	[int]   $RefineHaloLayers    = 2,       # extra coarse-grid layers around the moving frame
	[double]$TensionCutoffPa     = 3e9,     # EOS tension cutoff (Pa)
	[double]$DensityFloorFrac    = 0.001    # density floor as fraction of rho0 (0 = disabled)
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Exe))
{ throw "Executable not found: $Exe"
}
if (-not (Test-Path $Mesh))
{ throw "FE tool mesh not found: $Mesh"
}

# -- output directory --
if ([string]::IsNullOrWhiteSpace($ResultsDir)) {
	$stamp = Get-Date -Format "yyyyMMdd-HHmm"
	$ResultsDir = Join-Path (Join-Path "results\baseline" $stamp) "model5_fe_tool_500k"
}
New-Item -ItemType Directory -Force -Path $ResultsDir | Out-Null
$env:MFREE_RESULTS_DIR   = $ResultsDir
$env:MFREE_CLEAN_RESULTS = "1"

# -- geometry / physics --
$env:MFREE_CUTTING_SPEED_M_MIN    = "100"   # m/min  (1.67 m/s)
$env:MFREE_NBOX                   = "61"    # particle layers through thickness
$env:MFREE_FEED_PER_REV_MM        = "0.2"
$env:MFREE_BASE_TARGET_FEED_MM    = "0.1"
$env:MFREE_WORKPIECE_THICKNESS_MM = "0.5"

# -- dynamic refinement frame --
$env:MFREE_REFINE_DEPTH_FACTOR    = "$RefineDepthFactor"
$env:MFREE_REFINE_FRAME_WIDTH_MM  = "$RefineFrameWidthMm"
$env:MFREE_REFINE_HALO_LAYERS     = "$RefineHaloLayers"

# -- FE tool (deformable, explicit, thermally + mechanically coupled) --
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
$env:MFREE_FE_TOOL_FIX_TAGS        = "114"     # rear-face Dirichlet

# Explicit FE tool time integration inside the SPH contact loop
$env:MFREE_DEFORMABLE_FE_TOOL                    = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT           = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "100"
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS -ErrorAction SilentlyContinue
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS  -ErrorAction SilentlyContinue
$env:MFREE_FE_TOOL_RAYLEIGH_A0     = "0"
$env:MFREE_FE_TOOL_RAYLEIGH_A1     = "0"

# -- thermal contact --
$env:MFREE_THERMAL_H_FULL          = "1000000"  # W/m^2*K  full-pressure contact
$env:MFREE_THERMAL_H_SEP           = "10000"    # W/m^2*K  partial / air-gap contact
$env:MFREE_THERMAL_P_REF           = "1e9"      # Pa       reference pressure

$env:MFREE_THERMAL_FRAC_WP         = "0.8"      # friction heat fraction -> workpiece
$env:MFREE_THERMAL_FRAC_TOOL       = "0.2"      # friction heat fraction -> tool
$env:MFREE_THERMAL_MAX_DT_PER_STEP = "5"        # K per step limiter

# -- timestep estimator --
$env:MFREE_TIMESTEP_PRINT             = "1"
$env:MFREE_TIMESTEP_WP_MECH_SAFETY    = "0.25"
$env:MFREE_TIMESTEP_WP_THERM_SAFETY   = "0.20"
$env:MFREE_TIMESTEP_TOOL_MECH_SAFETY  = "0.90"
$env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "1.00"
$env:MFREE_TIMESTEP_INTERFACE_SAFETY  = "0.50"

$env:MFREE_T_FINAL_SCALE = "1.0"

# -- tensile instability control --
$env:MFREE_TENSION_CUTOFF     = "$TensionCutoffPa"
$env:MFREE_DENSITY_FLOOR_FRAC = "$DensityFloorFrac"

# -- run control --
$env:MFREE_MAX_STEPS = "$MaxSteps"

$outputFreq = [Math]::Max(1, [int]($MaxSteps / $OutputFrames))
$env:MFREE_OUTPUT_FREQ                   = "$outputFreq"
$env:MFREE_LOG_TIME_STEP_DATA_EVERY_STEP = "0"   # energy CSV at output_freq only

# -- logging --
$env:MFREE_LOG_THERMAL       = "1"   # cutting_thermal.csv
$env:MFREE_LOG_METRICS       = "1"   # cutting_metrics.csv
$env:MFREE_LOG_ENERGY        = "1"   # cutting_energy.csv
$env:MFREE_LOG_VTK_WORKPIECE = "1"   # out_NNNNNN.vtk
$env:MFREE_LOG_VTK_FE_TOOL   = "1"   # fe_tool_NNNNNN.vtk
$env:MFREE_LOG_VTK_TOOL      = "0"   # rigid tool VTK not used

# -- startup summary --
$dt_approx  = 4.678e-10
$t_total_us = [Math]::Round($MaxSteps * $dt_approx * 1e6, 1)
$cut_mm     = [Math]::Round($MaxSteps * $dt_approx * (100.0 / 60.0) * 1000, 3)

Write-Host ""
Write-Host "============================================================"
Write-Host " Model 5: Dynamic multi-resolution + deformable FE tool"
Write-Host "   100 m/min, nbox=61"
Write-Host "   MaxSteps     = $MaxSteps"
Write-Host "   dt (approx)  = $dt_approx s"
Write-Host "   t_total      ~ $t_total_us us"
Write-Host "   cut distance ~ $cut_mm mm"
Write-Host "   VTK frames   = $OutputFrames  (every $outputFreq steps)"
Write-Host "   Refinement   = depth_factor $RefineDepthFactor, width ${RefineFrameWidthMm} mm, halo $RefineHaloLayers layers"
Write-Host "   TensionCutoff= $TensionCutoffPa Pa  DensityFloor= $DensityFloorFrac * rho0"
Write-Host "   FE tool mesh = $Mesh"
Write-Host "   Results      -> $ResultsDir"
Write-Host "============================================================"
Write-Host ""

# -- preprocess pass --
Write-Host "--- Preprocess / geometry check (Model 5) ---"
$env:MFREE_PREPROCESS_ONLY = "1"
& $Exe -m 5
if ($LASTEXITCODE -ne 0)
{ throw "Preprocess failed (exit $LASTEXITCODE)"
}

# -- production run --
Write-Host ""
Write-Host "--- Starting $MaxSteps-step production run (Model 5) ---"
$env:MFREE_PREPROCESS_ONLY = "0"
$env:MFREE_CLEAN_RESULTS   = "0"   # Don't clean after preprocess
$startTime = Get-Date
& $Exe -m 5
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
Write-Host "   $ResultsDir\cutting_metrics.csv"
Write-Host "============================================================"

$global:LASTEXITCODE = 0
