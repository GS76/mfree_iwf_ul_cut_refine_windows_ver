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
$env:MFREE_DT_SCALE = "0.1"
$env:MFREE_T_FINAL_SCALE = "5.840165e-3"

$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS = "50"
$env:MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS = "20"

$env:MFREE_FE_TOOL_RAYLEIGH_A0 = "0"
$env:MFREE_FE_TOOL_RAYLEIGH_A1 = "0"
$env:MFREE_FE_TOOL_HARD_FAIL_ON_INVALID = "0"

$env:MFREE_FE_TOOL_RHO = "14500"
$env:MFREE_FE_TOOL_CP = "2"
$env:MFREE_FE_TOOL_K = "80"
$env:MFREE_FE_TOOL_E = "6e11"
$env:MFREE_FE_TOOL_NU = "0.22"
$env:MFREE_FE_TOOL_ALPHA = "4.5e-6"

$env:MFREE_FE_TOOL_FIX_TAGS = "114"

.\build\Release\mfree_iwf.exe -m 3

Pop-Location

