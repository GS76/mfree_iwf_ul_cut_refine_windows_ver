# run_model5_fe_tool.ps1
#
# Purpose
# -------
# Model 5: SPH workpiece (dynamic refinement) with fully coupled deformable FE tool.
# Runs a long-duration cutting simulation at 100 m/min with thermal and mechanical
# FE tool coupling to reach thermal quasi-steady state at the tool-workpiece interface.
# Default parameters are tuned for investigating localized heating near the contact zone.
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
#   .\scripts\run_model5_fe_tool.ps1 -MaxSteps 200000 -OutputFrames 600 -LogEveryStepData
#
param(
	[string]$Exe          = ".\build\Release\mfree_iwf.exe",
	[string]$Mesh                = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh",
	[string]$ResultsDir          = "",
	[int]   $MaxSteps            = 500000,  # steps to run
	[int]   $OutputFrames        = 300,     # VTK snapshots + CSV rows (denser sampling for hotspot tracking)
	[double]$RefineDepthFactor   = 1.0,     # moving-frame depth = factor * target cut/feed depth
	[double]$RefineFrameWidthMm  = 0.40,    # moving-frame width ahead of tool, mm
	[int]   $RefineHaloLayers    = 0,       # extra coarse-grid layers around the moving frame
	[double]$TensionCutoffPa     = 3e9,     # EOS tension cutoff (Pa)
	[double]$DensityFloorFrac    = 0.001,   # density floor as fraction of rho0 (0 = disabled)
	[double]$ThermalHFull        = 1e6,     # W/m^2*K full-pressure contact
	[double]$ThermalHSep         = 1e4,     # W/m^2*K partial / air-gap contact
	[double]$ThermalPRef         = 1e9,     # Pa thermal reference pressure
	[double]$ThermalFracWp       = 0.8,     # friction heat partition to workpiece
	[double]$ThermalFracTool     = 0.2,     # friction heat partition to tool
	[double]$ThermalMaxDtPerStepK = 2.0,    # K/step limiter tightened for hotspot diagnostics
	[double]$RhoPseFloorFrac     = 0.05,    # low-density cap used in PSE/Brookshaw terms
	[double]$ThermalSkipFrac     = 0.5,     # skip conduction if rho < this * rho0
	[double]$WorkpieceTempWarnK  = 1200.0,  # warning threshold for hotspot tracking
	[switch]$LogEveryStepData
)

$ErrorActionPreference = "Stop"
function Test-InWarpSession {
	if (-not [string]::IsNullOrWhiteSpace($env:TERM_PROGRAM) -and $env:TERM_PROGRAM -match "Warp") { return $true }
	if (-not [string]::IsNullOrWhiteSpace($env:WARP_IS_LOCAL_SHELL_SESSION) -and $env:WARP_IS_LOCAL_SHELL_SESSION -eq "1") { return $true }
	$warpEnv = Get-ChildItem Env:WARP_* -ErrorAction SilentlyContinue | Select-Object -First 1
	return ($null -ne $warpEnv)
}

$allowWarpSim = ($env:MFREE_ALLOW_WARP_SIM -eq "1")
if ((Test-InWarpSession) -and -not $allowWarpSim) {
	throw "Simulation execution is blocked inside Warp for this repository. Open an external Windows PowerShell session and re-run from repo root. Override only when intentional by setting MFREE_ALLOW_WARP_SIM=1."
}

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

# -- stability monitoring (for NaN/Inf detection and adaptive timestep) --
$env:MFREE_ENABLE_STABILITY_MONITOR        = "1"      # Enable stability checks for Model 5
$env:MFREE_MAX_VELOCITY_FACTOR              = "0.5"    # Max velocity as fraction of sound speed
$env:MFREE_ENERGY_CLOSURE_THRESHOLD         = "10.0"   # Warning threshold for energy closure (%)
$env:MFREE_ENERGY_CLOSURE_CRITICAL          = "50.0"   # Critical threshold for timestep reduction (%)
$env:MFREE_MIN_TIMESTEP                     = "1e-12"  # Minimum viable dt before termination (s)
$env:MFREE_MAX_TIMESTEP_REDUCTIONS          = "10"     # Max consecutive reductions before abort
$env:MFREE_STABILITY_VALIDATION_FREQ        = "1"      # Validate every N steps (1 = every step)
$env:MFREE_TEMP_MIN_K                       = "200"    # Minimum physically reasonable temperature
$env:MFREE_TEMP_MAX_K                       = "5000"   # Maximum physically reasonable temperature

# -- tensile instability monitoring (Issue #21) --
$env:MFREE_ENABLE_TENSILE_MONITORING        = "1"      # Enable σW'' tensile instability detection
$env:MFREE_TENSILE_THRESHOLD_RATIO          = "0.10"   # Warn if >10% particles unstable
$env:MFREE_MGHN_ADAPTIVE_EPS                = "1"      # Enable adaptive Monaghan artificial stress
$env:MFREE_MGHN_EPS_MIN                     = "0.0"    # Min epsilon
$env:MFREE_MGHN_EPS_MAX                     = "1.0"    # Max epsilon

# -- thermal contact --
$env:MFREE_THERMAL_H_FULL          = "$ThermalHFull"
$env:MFREE_THERMAL_H_SEP           = "$ThermalHSep"
$env:MFREE_THERMAL_P_REF           = "$ThermalPRef"

$env:MFREE_THERMAL_FRAC_WP         = "$ThermalFracWp"
$env:MFREE_THERMAL_FRAC_TOOL       = "$ThermalFracTool"
$env:MFREE_THERMAL_MAX_DT_PER_STEP = "$ThermalMaxDtPerStepK"

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
$env:MFREE_RHO_PSE_FLOOR_FRAC = "$RhoPseFloorFrac"
$env:MFREE_THERMAL_SKIP_FRAC  = "$ThermalSkipFrac"

# -- run control --
$env:MFREE_MAX_STEPS = "$MaxSteps"

$outputFreq = [Math]::Max(1, [int]($MaxSteps / $OutputFrames))
$env:MFREE_OUTPUT_FREQ                   = "$outputFreq"
if ($LogEveryStepData) {
	$env:MFREE_LOG_TIME_STEP_DATA_EVERY_STEP = "1"
} else {
	$env:MFREE_LOG_TIME_STEP_DATA_EVERY_STEP = "0"
}

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
Write-Host "   Thermal BC   = h_full $ThermalHFull, h_sep $ThermalHSep, p_ref $ThermalPRef"
Write-Host "   Fric split   = wp $ThermalFracWp, tool $ThermalFracTool, dT limiter ${ThermalMaxDtPerStepK} K/step"
Write-Host "   Thermal guard= rho_pse_floor $RhoPseFloorFrac, thermal_skip $ThermalSkipFrac"
Write-Host "   Log cadence  = every-step data: $([bool]$LogEveryStepData)"
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
function Get-CsvColumnMaximum {
	param(
		[string]$CsvPath,
		[string]$ColumnName
	)
	if (-not (Test-Path $CsvPath)) { return $null }
	$rows = Import-Csv -Path $CsvPath
	if (($null -eq $rows) -or ($rows.Count -eq 0)) { return $null }
	if (-not ($rows[0].PSObject.Properties.Name -contains $ColumnName)) { return $null }
	$maxValue = $null
	foreach ($row in $rows) {
		$raw = $row.$ColumnName
		if ([string]::IsNullOrWhiteSpace($raw)) { continue }
		try {
			$v = [double]::Parse($raw, [System.Globalization.CultureInfo]::InvariantCulture)
		} catch {
			continue
		}
		if ([double]::IsNaN($v) -or [double]::IsInfinity($v)) { continue }
		if (($null -eq $maxValue) -or ($v -gt $maxValue)) { $maxValue = $v }
	}
	return $maxValue
}

$thermalCsv = Join-Path $ResultsDir "cutting_thermal.csv"
$wpTmax = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "wp_Tmax"
$toolTmax = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "tool_Tmax"
if ($null -ne $wpTmax) {
	Write-Host (" Thermal maxima: wp_Tmax={0:F2} K, tool_Tmax={1:F2} K" -f $wpTmax, $toolTmax)
	if ($wpTmax -ge $WorkpieceTempWarnK) {
		Write-Host (" WARNING: wp_Tmax reached {0:F2} K (>= warn threshold {1:F2} K)" -f $wpTmax, $WorkpieceTempWarnK)
	}
}

# -- post-run energy validation --
$energyCsv = Join-Path $ResultsDir "cutting_energy.csv"
if (Test-Path $energyCsv) {
	$rows = Import-Csv -Path $energyCsv
	if ($rows.Count -gt 0) {
		# Check last row for energy closure residual
		$lastRow = $rows[$rows.Count - 1]
		$closureResidual = $null
		if ($lastRow.PSObject.Properties.Name -contains "closure_residual_pct") {
			$raw = $lastRow.closure_residual_pct
			if (-not [string]::IsNullOrWhiteSpace($raw)) {
				try {
					$closureResidual = [double]::Parse($raw, [System.Globalization.CultureInfo]::InvariantCulture)
				} catch {}
			}
		}
		
		$maxClosureResidual = $null
		foreach ($row in $rows) {
			$raw = $row.closure_residual_pct
			if ([string]::IsNullOrWhiteSpace($raw)) { continue }
			try {
				$v = [double]::Parse($raw, [System.Globalization.CultureInfo]::InvariantCulture)
				if ([double]::IsNaN($v) -or [double]::IsInfinity($v)) { continue }
				if (($null -eq $maxClosureResidual) -or ($v -gt $maxClosureResidual)) {
					$maxClosureResidual = $v
				}
			} catch {}
		}
		
		if ($null -ne $maxClosureResidual) {
			Write-Host ""
			Write-Host (" Energy closure residual: max = {0:F2}%, final = {1:F2}%" -f $maxClosureResidual, $(if ($null -ne $closureResidual) { $closureResidual } else { "N/A" }))
			if ($maxClosureResidual -gt 50.0) {
				Write-Host " WARNING: Energy closure residual exceeded 50% - simulation may have experienced numerical instability!"
			} elseif ($maxClosureResidual -gt 10.0) {
				Write-Host " WARNING: Energy closure residual exceeded 10% - recommend reviewing timestep and stability settings."
			}
		}
		
		# Check for NaN or Inf in any column
		$hasInvalidValues = $false
		foreach ($col in $rows[0].PSObject.Properties.Name) {
			foreach ($row in $rows) {
				$val = $row.$col
				if ($val -match "NaN|Infinity" -or [string]::IsNullOrEmpty($val)) {
					$hasInvalidValues = $true
					break
				}
				try {
					$v = [double]::Parse($val, [System.Globalization.CultureInfo]::InvariantCulture)
					if ([double]::IsNaN($v) -or [double]::IsInfinity($v)) {
						$hasInvalidValues = $true
						break
					}
				} catch {
					# Non-numeric columns are fine
				}
			}
			if ($hasInvalidValues) { break }
		}
		if ($hasInvalidValues) {
			Write-Host ""
			Write-Host " WARNING: cutting_energy.csv contains NaN or Inf values - simulation encountered numerical instability!"
		}
	}
}

$global:LASTEXITCODE = 0
