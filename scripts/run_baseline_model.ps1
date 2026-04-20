param(
	[int]$Model = 1,
	[string]$Label = "run",
	[string]$BaselineRoot = "results\\baseline",
	[string]$Exe = ".\\build\\Release\\mfree_iwf.exe"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Exe)) { throw "missing executable: $Exe" }
if (-not (Test-Path ".git")) { throw "run from repo root (missing .git)" }

$ts = Get-Date -Format "yyyyMMdd-HHmm"
$base = Join-Path $BaselineRoot $ts
$outDir = Join-Path $base $Label
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$env:MFREE_RESULTS_DIR = $outDir
$env:MFREE_CLEAN_RESULTS = "1"

$gitHead = ""
try { $gitHead = (git rev-parse HEAD).Trim() } catch { $gitHead = "" }

$envDumpPath = Join-Path $outDir "run_env.txt"
Get-ChildItem Env: | Sort-Object Name | ForEach-Object { "$($_.Name)=$($_.Value)" } | Set-Content -Path $envDumpPath -Encoding UTF8

$metaPath = Join-Path $outDir "run_meta.txt"
@(
	"timestamp=$ts"
	"label=$Label"
	"model=$Model"
	"exe=$Exe"
	"git_head=$gitHead"
) | Set-Content -Path $metaPath -Encoding UTF8

& $Exe -m $Model

