param(
	[ValidateSet("tool","workpiece","both")]
	[string]$PrimaryMovingBody = "tool",
	[double]$CoupledRatio = [double]::NaN,
	[switch]$NoRigidTool = $false,
	[switch]$PreprocessOnly = $true,
	[int]$MaxSteps = 0,
	[switch]$UseMeshForContact = $false
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

$env:MFREE_FEED_PER_REV_MM = "0.2"
$env:MFREE_BASE_TARGET_FEED_MM = "0.1"
$env:MFREE_WORKPIECE_THICKNESS_MM = "0.5"

$env:MFREE_FE_TOOL_MSH = $mesh
if ($NoRigidTool) {
	$env:MFREE_NO_RIGID_TOOL = "1"
	$env:MFREE_CONTACT_MU = "0.35"
	$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
} else {
	Remove-Item Env:\MFREE_NO_RIGID_TOOL -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_CONTACT_MU -ErrorAction SilentlyContinue
	if ($UseMeshForContact) { $env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1" } else { Remove-Item Env:\MFREE_USE_FE_TOOL_FOR_CONTACT -ErrorAction SilentlyContinue }
}

$env:MFREE_COUPLED_MOTION = "1"
$env:MFREE_PRIMARY_MOVING_BODY = $PrimaryMovingBody
if ([double]::IsNaN($CoupledRatio)) { Remove-Item Env:\MFREE_COUPLED_MOTION_RATIO -ErrorAction SilentlyContinue } else { $env:MFREE_COUPLED_MOTION_RATIO = "$CoupledRatio" }

if ($PreprocessOnly) { $env:MFREE_PREPROCESS_ONLY = "1" } else { Remove-Item Env:\MFREE_PREPROCESS_ONLY -ErrorAction SilentlyContinue }

if ($MaxSteps -gt 0) { $env:MFREE_MAX_STEPS = "$MaxSteps" } else { Remove-Item Env:\MFREE_MAX_STEPS -ErrorAction SilentlyContinue }

foreach ($m in 1..4) {
	$env:MFREE_RESULTS_DIR = "results\validate_model_$m"
	$env:MFREE_CLEAN_RESULTS = "1"
	& $exe -m $m
}
