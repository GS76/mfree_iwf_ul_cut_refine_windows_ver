$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\\..")
Push-Location $repoRoot

$env:MFREE_FE_TOOL_MSH = "snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
$env:MFREE_FE_TOOL_ALIGN_CENTER = "0"

$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_ALPHA = "0.01"
$env:MFREE_DEFORMABLE_FE_TOOL = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT = "1"

$env:MFREE_PREPROCESS_ONLY = "0"

# Use the coupled timestep estimator directly to determine the largest
# admissible global step for this SPH workpiece + FE tool configuration.
# Set this below 1.0 only when you want an additional safety reduction.
$env:MFREE_DT_SCALE = "1.0"

# Print all estimator limits and the controlling limiter at startup.
$env:MFREE_TIMESTEP_PRINT = "1"
$env:MFREE_TIMESTEP_WP_MECH_SAFETY = "0.25"
$env:MFREE_TIMESTEP_WP_THERM_SAFETY = "0.20"
$env:MFREE_TIMESTEP_TOOL_MECH_SAFETY = "0.90"
$env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "1.00"
$env:MFREE_TIMESTEP_INTERFACE_SAFETY = "0.50"
$env:MFREE_TIMESTEP_INTERFACE_AREA_FACTOR = "1.0"

# Allow a 100000-step run. MFREE_T_FINAL_SCALE is kept high enough that
# the executable stops by MFREE_MAX_STEPS instead of final simulation time.
$env:MFREE_MAX_STEPS = "100000"
$env:MFREE_T_FINAL_SCALE = "0.05"

# Enable per-step/cumulative energy accounting output: results/cutting_energy.csv
$env:MFREE_LOG_ENERGY = "1"

# Let the solver derive explicit FE-tool substeps from the estimated timestep
# and the FE tool critical limits. Raise this cap only if the startup report
# or stability checks show that more substeps are needed.
Remove-Item Env:MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS -ErrorAction SilentlyContinue
Remove-Item Env:MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS -ErrorAction SilentlyContinue
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "100"

$env:MFREE_FE_TOOL_RAYLEIGH_A0 = "0"
$env:MFREE_FE_TOOL_RAYLEIGH_A1 = "0"
$env:MFREE_FE_TOOL_HARD_FAIL_ON_INVALID = "0"

$env:MFREE_FE_TOOL_RHO = "14500"
$env:MFREE_FE_TOOL_CP = "20"
$env:MFREE_FE_TOOL_K = "80"
$env:MFREE_FE_TOOL_E = "6e11"
$env:MFREE_FE_TOOL_NU = "0.22"
$env:MFREE_FE_TOOL_ALPHA = "4.5e-6"

$env:MFREE_FE_TOOL_FIX_TAGS = "114"

.\build\Release\mfree_iwf.exe -m 3

Pop-Location
