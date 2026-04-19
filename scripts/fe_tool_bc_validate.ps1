param(
	[switch]$NoRigidTool = $true,
	[double]$CuttingSpeed_m_min = 100.0,
	[int]$TopTag = 110,
	[int]$RearTag = 114,
	[double]$Ambient_C = 25.0,
	[switch]$AnchorUx = $true
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_CLEAN_RESULTS = "1"
$env:MFREE_FE_TOOL_MSH = $mesh
$env:MFREE_CUTTING_SPEED_M_MIN = "$CuttingSpeed_m_min"

$env:MFREE_FE_BC_VALIDATE = "1"
$env:MFREE_FE_BC_TOP_TAG = "$TopTag"
$env:MFREE_FE_BC_REAR_TAG = "$RearTag"
$env:MFREE_FE_BC_AMBIENT_C = "$Ambient_C"
if ($AnchorUx) { $env:MFREE_FE_BC_ANCHOR_UX = "1" } else { $env:MFREE_FE_BC_ANCHOR_UX = "0" }

if ($NoRigidTool) {
	$env:MFREE_NO_RIGID_TOOL = "1"
	$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
	$env:MFREE_CONTACT_MU = "0.35"
} else {
	Remove-Item Env:\MFREE_NO_RIGID_TOOL -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_USE_FE_TOOL_FOR_CONTACT -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_CONTACT_MU -ErrorAction SilentlyContinue
}

foreach ($m in 1..4) {
	$env:MFREE_RESULTS_DIR = "results\fe_bc_validate\model_$m"
	& $exe -m $m
}

