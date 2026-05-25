param(
	[string]$ResultsDir = "results\model1_fe_tool_thermal",
	[double]$CuttingSpeed_m_min = 100.0,
	[int]$MaxSteps = 200,
	[int]$OutputSteps = 10,
	[string]$FeToolMesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh",
	[double]$FeToolRho = 14500.0,
	[double]$FeToolCp = 2.0,
	[double]$FeToolK = 80.0
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"

$env:MFREE_RESULTS_DIR = $ResultsDir
$env:MFREE_CLEAN_RESULTS = "1"
$env:MFREE_PREPROCESS_ONLY = "0"

$env:MFREE_FE_TOOL_MSH = $FeToolMesh
$env:MFREE_NO_RIGID_TOOL = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_DEFORMABLE_FE_TOOL = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "25"

$env:MFREE_CUTTING_SPEED_M_MIN = "$CuttingSpeed_m_min"

$env:MFREE_FE_TOOL_RHO = "$FeToolRho"
$env:MFREE_FE_TOOL_CP = "$FeToolCp"
$env:MFREE_FE_TOOL_K = "$FeToolK"

$env:MFREE_FE_TOOL_FIX_Y_TAGS = "110,114"
$env:MFREE_FE_TOOL_ANCHOR_UX = "1"
$env:MFREE_FE_TOOL_ANCHOR_TAG = "114"

$env:MFREE_LOG_THERMAL = "1"
$env:MFREE_LOG_METRICS = "1"
$env:MFREE_LOG_VTK_WORKPIECE = "1"
$env:MFREE_LOG_VTK_TOOL = "0"
$env:MFREE_LOG_VTK_FE_TOOL = "1"

$env:MFREE_MAX_STEPS = "$MaxSteps"
$env:MFREE_NUM_PRINT = "$OutputSteps"
$freq = 1
if ($OutputSteps -gt 0) { $freq = [Math]::Max(1, [int]($MaxSteps / $OutputSteps)) }
$env:MFREE_OUTPUT_FREQ = "$freq"

& $exe -m 1
