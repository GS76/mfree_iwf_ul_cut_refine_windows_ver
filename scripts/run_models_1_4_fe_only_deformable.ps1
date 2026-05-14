param(
	[int]$MaxSteps = 2000,
	[int]$OutputSteps = 20,
	[double]$CuttingSpeed_m_min = 100.0
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

$env:MFREE_FE_TOOL_MSH = $mesh
$env:MFREE_PREPROCESS_ONLY = "0"
$env:MFREE_NO_RIGID_TOOL = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_MU = "0.35"

$env:MFREE_CUTTING_SPEED_M_MIN = "$CuttingSpeed_m_min"

$env:MFREE_DEFORMABLE_FE_TOOL = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "25"

$env:MFREE_LOG_THERMAL = "1"
$env:MFREE_LOG_VTK_WORKPIECE = "1"
$env:MFREE_LOG_VTK_TOOL = "0"
$env:MFREE_LOG_VTK_FE_TOOL = "1"

$env:MFREE_MAX_STEPS = "$MaxSteps"
$env:MFREE_NUM_PRINT = "$OutputSteps"
$freq = 1
if ($OutputSteps -gt 0) { $freq = [Math]::Max(1, [int]($MaxSteps / $OutputSteps)) }
$env:MFREE_OUTPUT_FREQ = "$freq"

foreach ($m in 1..4) {
	$env:MFREE_RESULTS_DIR = "results\fe_only_deformable_v100\model_$m"
	$env:MFREE_CLEAN_RESULTS = "1"
	& $exe -m $m
}
