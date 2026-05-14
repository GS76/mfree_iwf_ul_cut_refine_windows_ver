param(
	[switch]$NoRigidTool = $true,
	[switch]$AutoCorrect = $false
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_CLEAN_RESULTS = "1"
$env:MFREE_GEOM_VALIDATE = "1"
$env:MFREE_GEOM_CLEARANCE_MM = "0.2"
$env:MFREE_GEOM_CLEARANCE_TOL_MM = "0.001"
$env:MFREE_GEOM_TANGENCY_TOL_MM = "0.000001"

if ($AutoCorrect) { $env:MFREE_GEOM_AUTO_CORRECT = "1" } else { Remove-Item Env:\MFREE_GEOM_AUTO_CORRECT -ErrorAction SilentlyContinue }

$env:MFREE_FE_TOOL_MSH = $mesh

if ($NoRigidTool) {
	$env:MFREE_NO_RIGID_TOOL = "1"
	$env:MFREE_CONTACT_MU = "0.35"
	$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
} else {
	Remove-Item Env:\MFREE_NO_RIGID_TOOL -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_CONTACT_MU -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_USE_FE_TOOL_FOR_CONTACT -ErrorAction SilentlyContinue
}

foreach ($m in 1..4) {
	$env:MFREE_RESULTS_DIR = "results\geom_validate_model_$m"
	& $exe -m $m
}
