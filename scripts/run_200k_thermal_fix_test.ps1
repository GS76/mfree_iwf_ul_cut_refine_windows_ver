# run_200k_thermal_fix_test.ps1
#
# Purpose
# -------
# Long-run (default 500k-step) verification to check whether high-temperature
# issues still occur inside the SPH workpiece while using the Phase-2 thermal
# and stability fixes:
#   1. Density floor added to heat_conduction_brookshaw (thermal.cpp)
#   2. T_t copied in adaptivity::copy_dad_to_son (adaptivity.cpp)
#   3. Low-density exclusion in PSE and Brookshaw conduction (thermal.cpp)
#   4. Plasticity: invalid-density check, extreme-stress skip, and graceful radial-return handling (plasticity.cpp)
#   5. Leapfrog: reset rho_t and zero deviatoric stress for density-floored particles (leap_frog.cpp)
#   6. T_t diagnostic field added to VTK output (vtk_writer.cpp)
#
# This run uses Model 3 (dynamic refinement) to exercise adaptivity paths
# and enables density floor to protect low-density void particles.
#
# Usage (from repo root):
#   .\scripts\run_200k_thermal_fix_test.ps1
#   .\scripts\run_200k_thermal_fix_test.ps1 -MaxSteps 500000
#   .\scripts\run_200k_thermal_fix_test.ps1 -MaxSteps 50000  # quick smoke
#   .\scripts\run_200k_thermal_fix_test.ps1 -PreserveExistingResults
#
param(
	[string]$Exe = ".\build\Release\mfree_iwf.exe",
	[string]$Mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh",
	[string]$ResultsDir = "",
	[int]   $MaxSteps   = 500000,
	[int]   $OutputFrames = 100,
	[double]$WorkpieceTempWarnK = 2500.0,
	[switch]$PreserveExistingResults
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
# Default to versioned results layout from docs/development_workflow.md:
# results/baseline/<YYYYMMDD-HHMM>/<label>/
if ([string]::IsNullOrWhiteSpace($ResultsDir)) {
	$stamp = Get-Date -Format "yyyyMMdd-HHmm"
	$ResultsDir = Join-Path (Join-Path "results\baseline" $stamp) "thermal_fix_500k_high_temp"
}

# ── Executable ────────────────────────────────────────────────────────────────
if (-not (Test-Path $Exe)) { throw "Executable not found: $Exe" }
if (-not (Test-Path $Mesh)) { throw "FE tool mesh not found: $Mesh" }

# ── Output directory ───────────────────────────────────────────────────────────
if ($PreserveExistingResults -and (Test-Path $ResultsDir)) {
	$existingItems = Get-ChildItem -Path $ResultsDir -Force -ErrorAction SilentlyContinue
	if ($null -ne $existingItems) {
		$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
		$parent = Split-Path -Parent $ResultsDir
		$leaf = Split-Path -Leaf $ResultsDir
		if ([string]::IsNullOrWhiteSpace($parent)) { $parent = "." }
		$archiveDir = Join-Path $parent "${leaf}_${stamp}"
		Move-Item -Path $ResultsDir -Destination $archiveDir
		Write-Host "Archived existing results to $archiveDir"
	}
}
New-Item -ItemType Directory -Force -Path $ResultsDir | Out-Null
$env:MFREE_RESULTS_DIR   = $ResultsDir
$env:MFREE_CLEAN_RESULTS = "0"

# ── Geometry / physics ────────────────────────────────────────────────────────
$env:MFREE_CUTTING_SPEED_M_MIN    = "100"    # 100 m/min (1.67 m/s)
$env:MFREE_NBOX                   = "61"     # dx ~ 8.3 um
$env:MFREE_FEED_PER_REV_MM        = "0.2"
$env:MFREE_BASE_TARGET_FEED_MM    = "0.1"
$env:MFREE_WORKPIECE_THICKNESS_MM = "0.5"

# ── Dynamic refinement (Model 3) ─────────────────────────────────────────────
$env:MFREE_REFINE_DEPTH_FACTOR    = "1.0"
$env:MFREE_REFINE_FRAME_WIDTH_MM  = "0.40"
$env:MFREE_REFINE_HALO_LAYERS     = "0"

# ── FE tool (deformable) ─────────────────────────────────────────────────────
$env:MFREE_FE_TOOL_MSH             = $Mesh
$env:MFREE_NO_RIGID_TOOL           = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_MU              = "0.35"

# WC-Co tool material
$env:MFREE_FE_TOOL_RHO   = "14500"   # kg/m^3
$env:MFREE_FE_TOOL_CP    = "200"     # J/kg*K
$env:MFREE_FE_TOOL_K     = "80"      # W/m*K
$env:MFREE_FE_TOOL_E     = "6e11"    # Pa
$env:MFREE_FE_TOOL_NU    = "0.22"
$env:MFREE_FE_TOOL_ALPHA = "4.5e-6"  # 1/K
$env:MFREE_FE_TOOL_FIX_TAGS = "114"   # rear-face Dirichlet at 298 K

# Explicit FE tool time integration
$env:MFREE_DEFORMABLE_FE_TOOL                  = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT         = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "100"
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS -ErrorAction SilentlyContinue
Remove-Item Env:\MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS -ErrorAction SilentlyContinue
$env:MFREE_FE_TOOL_RAYLEIGH_A0 = "0"
$env:MFREE_FE_TOOL_RAYLEIGH_A1 = "0"

# ── Thermal contact ───────────────────────────────────────────────────────────
$env:MFREE_THERMAL_H_FULL  = "1000000"  # W/m^2*K full-pressure contact
$env:MFREE_THERMAL_H_SEP   = "10000"    # W/m^2*K partial/air-gap contact
$env:MFREE_THERMAL_P_REF   = "1e9"      # Pa reference pressure
$env:MFREE_THERMAL_FRAC_WP   = "0.8"    # friction heat fraction -> workpiece
$env:MFREE_THERMAL_FRAC_TOOL = "0.2"    # friction heat fraction -> tool
$env:MFREE_THERMAL_MAX_DT_PER_STEP = "5"  # K per step limiter

# ── Timestep estimator ────────────────────────────────────────────────────────
$env:MFREE_TIMESTEP_PRINT           = "1"
$env:MFREE_TIMESTEP_WP_MECH_SAFETY  = "0.25"
$env:MFREE_TIMESTEP_WP_THERM_SAFETY = "0.20"
$env:MFREE_TIMESTEP_TOOL_MECH_SAFETY  = "0.90"
$env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "1.00"
$env:MFREE_TIMESTEP_INTERFACE_SAFETY  = "0.50"

# ── Tension cutoff & density floor (Phase 2 Fix 4 & thermal protection) ─────
$env:MFREE_TENSION_CUTOFF      = "3000000000"  # 3 GPa
$env:MFREE_DENSITY_FLOOR_FRAC = "0.001"        # rho_min = 0.1% rho0 (protects voids)
# Thermal protection thresholds (can be overridden via env)
$env:MFREE_RHO_PSE_FLOOR_FRAC  = "0.05"         # fraction of rho0 used to cap neighbor volumes in PSE/Brookshaw
$env:MFREE_THERMAL_SKIP_FRAC   = "0.5"          # skip conduction for particles with rho < this * rho0

# ── Run control ────────────────────────────────────────────────────────────────
$env:MFREE_MAX_STEPS = "$MaxSteps"
$outputFreq = [Math]::Max(1, [int]($MaxSteps / $OutputFrames))
$env:MFREE_OUTPUT_FREQ                   = "$outputFreq"
$env:MFREE_LOG_TIME_STEP_DATA_EVERY_STEP = "0"

# ── Logging (enable T_t diagnostic in VTK) ───────────────────────────────────
$env:MFREE_LOG_THERMAL       = "1"   # cutting_thermal.csv
$env:MFREE_LOG_METRICS       = "1"   # cutting_metrics.csv
$env:MFREE_LOG_ENERGY        = "1"   # cutting_energy.csv
$env:MFREE_LOG_VTK_WORKPIECE = "1"   # out_NNNNNN.vtk (now includes T_t)
$env:MFREE_LOG_VTK_FE_TOOL   = "1"   # fe_tool_NNNNNN.vtk
$env:MFREE_LOG_VTK_TOOL      = "0"   # rigid tool VTK not used

# ── Startup summary ───────────────────────────────────────────────────────────
$dt_approx  = 4.678e-10
$t_total_us = [Math]::Round($MaxSteps * $dt_approx * 1e6, 1)
$cut_mm     = [Math]::Round($MaxSteps * $dt_approx * (100.0 / 60.0) * 1000, 3)

Write-Host ""
Write-Host "============================================================"
Write-Host " Thermal Fix Verification - Long-Run High-Temperature Check"
Write-Host "   MaxSteps     = $MaxSteps"
Write-Host "   dt (approx)  = $dt_approx s"
Write-Host "   t_total      ~ $t_total_us us"
Write-Host "   cut distance ~ $cut_mm mm"
Write-Host "   VTK frames   = $OutputFrames (every $outputFreq steps)"
Write-Host "   wp_Tmax warn >= $WorkpieceTempWarnK K"
Write-Host "   Density floor = 0.001 * rho0 (protects voids)"
Write-Host "   Results      -> $ResultsDir"
Write-Host "============================================================"
Write-Host ""

# ── Preprocess pass (geometry check) ─────────────────────────────────────────
Write-Host "--- Preprocess / geometry check (Model 3) ---"
$env:MFREE_PREPROCESS_ONLY = "1"
$preLog = Join-Path $ResultsDir "preprocess.log"
Write-Host "Writing preprocess log to $preLog"
& $Exe -m 3 2>&1 | Tee-Object -FilePath $preLog
$preExit = $LASTEXITCODE
if ($preExit -ne 0) {
	Write-Host "Preprocess failed (exit $preExit). Last 200 lines of ${preLog}:"
	Get-Content $preLog -Tail 200
	throw "Preprocess failed (exit $preExit)"
}


# ── Production run ────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "--- Starting $MaxSteps-step production run (Model 3) ---"
$env:MFREE_PREPROCESS_ONLY = "0"
$env:MFREE_CLEAN_RESULTS   = "0"  # Don't clean after preprocess
$runLog = Join-Path $ResultsDir "run.log"
Write-Host "Writing runtime log to $runLog"
$startTime = Get-Date
& $Exe -m 3 2>&1 | Tee-Object -FilePath $runLog
$runExit = $LASTEXITCODE
$elapsed = (Get-Date) - $startTime
Set-Content -Path (Join-Path $ResultsDir "run_exit_code.txt") -Value $runExit

if ($runExit -ne 0) {
	Write-Host "Production run failed (exit $runExit). Last 200 lines of ${runLog}:"
	Get-Content $runLog -Tail 200
	if (Test-Path "plast_debug.txt") {
		Write-Host "plast_debug.txt (tail 200):"
		Get-Content "plast_debug.txt" -Tail 200
	}
	exit $runExit
}

# ── Post-run automated checks ─────────────────────────────────────────────────
Write-Host ""
Write-Host "============================================================"
Write-Host (" Run complete in {0:hh\:mm\:ss}" -f $elapsed)
Write-Host ""

# 1) VTK files and T_t presence
$vtkFiles = Get-ChildItem -Path (Join-Path $ResultsDir "out_*.vtk") -ErrorAction SilentlyContinue
if (($null -eq $vtkFiles) -or ($vtkFiles.Count -eq 0)) {
	Write-Host "WARNING: No out_*.vtk files found in $ResultsDir"
} else {
	$hasTt = $false
	foreach ($f in $vtkFiles) {
		if (Select-String -Pattern "SCALARS T_t" -Path $f -Quiet) { $hasTt = $true; break }
	}
	if (-not $hasTt) { Write-Host "WARNING: No 'SCALARS T_t' found in any out_*.vtk files" } else { Write-Host "VTK T_t field found in at least one file." }
}

# 2) CSV checks focused on thermal stability in the SPH workpiece
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

foreach ($csv in @("cutting_thermal.csv","cutting_metrics.csv","cutting_energy.csv")) {
	$path = Join-Path $ResultsDir $csv
	if (-not (Test-Path $path)) { Write-Host "ERROR: $csv missing in $ResultsDir"; continue }
	$lines = (Get-Content $path).Count
	if ($lines -lt 2) { Write-Host "WARNING: $csv looks empty ($lines lines)"; continue }
	Write-Host "$csv exists ($lines lines)"
	if (Select-String -Path $path -Pattern "NaN|Infinity|INF" -Quiet) {
		Write-Host "WARNING: $csv contains NaN/Inf entries"
	}
}

$thermalCsv = Join-Path $ResultsDir "cutting_thermal.csv"
$metricsCsv = Join-Path $ResultsDir "cutting_metrics.csv"

$wpTmaxThermal = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "wp_Tmax"
$wpTavgThermal = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "wp_Tavg"
$toolTmaxThermal = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "tool_Tmax"

if ($null -ne $wpTmaxThermal) {
	Write-Host ("Thermal CSV maxima: wp_Tmax={0:F2} K, wp_Tavg={1:F2} K, tool_Tmax={2:F2} K" -f $wpTmaxThermal, $wpTavgThermal, $toolTmaxThermal)
	if ($wpTmaxThermal -ge $WorkpieceTempWarnK) {
		Write-Host ("WARNING: wp_Tmax reached {0:F2} K (>= warn threshold {1:F2} K)" -f $wpTmaxThermal, $WorkpieceTempWarnK)
	}
} else {
	Write-Host "WARNING: Unable to compute thermal maxima from cutting_thermal.csv"
}

$wpTmaxMetrics = Get-CsvColumnMaximum -CsvPath $metricsCsv -ColumnName "wp_Tmax"
if ($null -ne $wpTmaxMetrics) {
	Write-Host ("Metrics CSV max wp_Tmax={0:F2} K" -f $wpTmaxMetrics)
	if ($wpTmaxMetrics -ge $WorkpieceTempWarnK) {
		Write-Host ("WARNING: metrics wp_Tmax reached {0:F2} K (>= warn threshold {1:F2} K)" -f $wpTmaxMetrics, $WorkpieceTempWarnK)
	}
}

$maxLoggedStep = Get-CsvColumnMaximum -CsvPath $thermalCsv -ColumnName "step"
if ($null -ne $maxLoggedStep) {
	$minExpectedStep = [Math]::Max(0, $MaxSteps - $outputFreq)
	Write-Host ("Thermal CSV max logged step = {0}" -f ([int]$maxLoggedStep))
	if ($maxLoggedStep -lt $minExpectedStep) {
		Write-Host ("WARNING: Max logged step ({0}) is below expected range (>= {1}); check for early termination" -f ([int]$maxLoggedStep), $minExpectedStep)
	}
}

# 3) plasticity debug checks
$plastPath = "plast_debug.txt"
if (Test-Path $plastPath) {
	$warnCount = (Select-String -Path $plastPath -Pattern "radial-return|Radial return|plastic|PLAST" -AllMatches | Measure-Object).Count
	if ($warnCount -gt 0) {
		Write-Host "WARNING: plast_debug.txt contains $warnCount warning entries. Showing tail 200:"
		Get-Content $plastPath -Tail 200
	} else {
		Write-Host "No plasticity warnings found in plast_debug.txt"
	}
} else {
	Write-Host "No plast_debug.txt found"
}

Write-Host "============================================================"
Write-Host "Automated checks complete. Inspect logs in $ResultsDir for details."
Write-Host "============================================================"

$global:LASTEXITCODE = 0
