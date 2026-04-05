$ErrorActionPreference = "Stop"

$env:MFREE_FE_TOOL_MSH = "snapshots/tool_plane_strain/meshes/tool_h_0.002mm.msh"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_DEFORMABLE_FE_TOOL = "1"
$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_FE_TOOL_ALIGN_CENTER = "0"

$env:MFREE_DEFORMABLE_TOOL_TOL = "0.01"
$env:MFREE_DEFORMABLE_TOOL_MAX_ITERS = "2000"
$env:MFREE_DEFORMABLE_TOOL_RELAX = "0.05"

$env:MFREE_FE_TOOL_RHO = "14500"
$env:MFREE_FE_TOOL_CP = "200"
$env:MFREE_FE_TOOL_K = "80"
$env:MFREE_FE_TOOL_E = "6e11"
$env:MFREE_FE_TOOL_NU = "0.22"
$env:MFREE_FE_TOOL_ALPHA = "4.5e-6"

$env:MFREE_FE_TOOL_FIX_TAGS = "114"

.\build\Release\mfree_iwf.exe -m 1

