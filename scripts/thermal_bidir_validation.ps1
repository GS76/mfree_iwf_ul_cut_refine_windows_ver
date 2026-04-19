param(
	[int]$MaxSteps = 200,
	[int]$NumPrint = 5,
	[switch]$NoRigidTool = $true
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

$env:MFREE_FE_TOOL_MSH = $mesh
$env:MFREE_PREPROCESS_ONLY = "0"
$env:MFREE_CLEAN_RESULTS = "1"
$env:MFREE_MAX_STEPS = "$MaxSteps"
$env:MFREE_NUM_PRINT = "$NumPrint"
$freq = 1
if ($NumPrint -gt 0) { $freq = [Math]::Max(1, [int]($MaxSteps / $NumPrint)) }
$env:MFREE_OUTPUT_FREQ = "$freq"
$env:MFREE_LOG_THERMAL = "1"
$env:MFREE_LOG_VTK_TOOL = "0"
$env:MFREE_LOG_VTK_FE_TOOL = "1"
$env:MFREE_LOG_VTK_WORKPIECE = "1"

$env:MFREE_THERMAL_H_SEP = "1e6"
$env:MFREE_THERMAL_H_FULL = "1e6"
$env:MFREE_THERMAL_P_REF = "1e6"
$env:MFREE_THERMAL_MAX_DT_PER_STEP = "50"

if ($NoRigidTool) {
	$env:MFREE_NO_RIGID_TOOL = "1"
	$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
	$env:MFREE_CONTACT_MU = "0"
} else {
	Remove-Item Env:\MFREE_NO_RIGID_TOOL -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_USE_FE_TOOL_FOR_CONTACT -ErrorAction SilentlyContinue
	Remove-Item Env:\MFREE_CONTACT_MU -ErrorAction SilentlyContinue
}

function RunCase([string]$CaseName, [double]$WpT0, [double]$ToolT0) {
	$env:MFREE_WP_T0 = "$WpT0"
	$env:MFREE_TOOL_T0 = "$ToolT0"
	foreach ($m in 1..4) {
		$env:MFREE_RESULTS_DIR = "results\thermal_bidir\$CaseName\model_$m"
		& $exe -m $m
	}
}

RunCase "wp_hot_tool_cold" 600 300
RunCase "wp_cold_tool_hot" 300 600
