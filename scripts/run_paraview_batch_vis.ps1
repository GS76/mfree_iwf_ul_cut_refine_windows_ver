param(
	[string]$FeVtk,
	[string]$TopCsv,
	[string]$RearCsv,
	[string]$OutDir,
	[string]$ModelLabel = "unknown",
	[double]$CuttingSpeed_m_min = 100.0,
	[int]$Width = 1920,
	[int]$Height = 1080,
	[double]$NodeGlyphScale = 0.0002,
	[double]$VelocityGlyphScale = 0.001,
	[switch]$SkipEdge = $false,
	[switch]$SkipVelocity = $false,
	[switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"

function Find-PvPython {
	$cmd = Get-Command pvpython -ErrorAction SilentlyContinue
	if ($null -ne $cmd) { return $cmd.Source }
	if ($env:PARAVIEW_HOME) {
		$p = Join-Path $env:PARAVIEW_HOME "bin\pvpython.exe"
		if (Test-Path $p) { return $p }
	}
	$candidates = @()
	$candidates += Get-ChildItem "C:\Program Files\ParaView*\bin\pvpython.exe" -ErrorAction SilentlyContinue
	$candidates += Get-ChildItem "C:\Program Files (x86)\ParaView*\bin\pvpython.exe" -ErrorAction SilentlyContinue
	if ($candidates.Count -gt 0) {
		return ($candidates | Sort-Object FullName -Descending | Select-Object -First 1).FullName
	}
	return $null
}

$pvpython = Find-PvPython
if (-not $pvpython) {
	Write-Error "pvpython.exe not found. Install ParaView or add pvpython to PATH. You can also set PARAVIEW_HOME to the ParaView install directory."
}

$scriptPath = Join-Path $PSScriptRoot "paraview_batch_vis.py"
if (-not (Test-Path $scriptPath)) { Write-Error "Missing script: $scriptPath" }

$argsList = @(
	$scriptPath,
	"--fe-vtk", $FeVtk,
	"--out-dir", $OutDir,
	"--model-label", $ModelLabel,
	"--cutting-speed-m-min", "$CuttingSpeed_m_min",
	"--width", "$Width",
	"--height", "$Height",
	"--node-glyph-scale", "$NodeGlyphScale",
	"--velocity-glyph-scale", "$VelocityGlyphScale"
)

if (-not $SkipEdge) {
	$argsList += @("--top-csv", $TopCsv, "--rear-csv", $RearCsv)
} else {
	$argsList += "--skip-edge"
}
if ($SkipVelocity) { $argsList += "--skip-velocity" }
if ($Verbose) { $argsList += "--verbose" }

& $pvpython @argsList
exit $LASTEXITCODE

