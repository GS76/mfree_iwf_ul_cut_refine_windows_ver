# run_image_extraction.ps1
# Wrapper for scripts/extract_results_image_info.py using persistent project requirements.

param(
	[Parameter(Mandatory = $true, Position = 0)]
	[string[]]$Inputs,
	[string]$PythonExe = "python",
	[string]$RequirementsFile = "config\\image_extraction_requirements.json",
	[string]$OutputJson = "results\\image_extract\\image_extract_summary.json",
	[string]$OutputCsv = "results\\image_extract\\image_extract_summary.csv",
	[string]$DebugDir = "results\\image_extract\\debug",
	[string]$Patterns,
	[switch]$Recursive,
	[string]$Roi,
	[switch]$RoiRelative,
	[string]$RoiPreset,
	[int]$DarkThreshold,
	[int]$EdgeThreshold,
	[int]$MinComponentArea,
	[double]$PresenceAreaRatioThreshold,
	[double]$PresenceEdgeRatioThreshold,
	[switch]$NoOcr,
	[string]$OcrLanguage,
	[switch]$PrettyJson,
	[switch]$StrictRequirements,
	[switch]$NoStrictRequirements
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $repoRoot

$cliScript = Join-Path $repoRoot "scripts\\extract_results_image_info.py"
if (-not (Test-Path $cliScript))
{
	throw "CLI script not found: $cliScript"
}
if (-not (Test-Path $RequirementsFile))
{
	throw "Requirements file not found: $RequirementsFile"
}

$argList = @($cliScript)
$argList += $Inputs
$argList += @("--requirements-file", $RequirementsFile)
$argList += @("--output-json", $OutputJson)
$argList += @("--output-csv", $OutputCsv)
$argList += @("--debug-dir", $DebugDir)

if ($PSBoundParameters.ContainsKey("Patterns"))
{
	$argList += @("--patterns", $Patterns)
}
if ($Recursive)
{
	$argList += "--recursive"
}
if ($Roi)
{
	$argList += @("--roi", $Roi)
}
if ($RoiRelative)
{
	$argList += "--roi-relative"
}
if ($PSBoundParameters.ContainsKey("RoiPreset"))
{
	$argList += @("--roi-preset", $RoiPreset)
}
if ($PSBoundParameters.ContainsKey("DarkThreshold"))
{
	$argList += @("--dark-threshold", $DarkThreshold)
}
if ($PSBoundParameters.ContainsKey("EdgeThreshold"))
{
	$argList += @("--edge-threshold", $EdgeThreshold)
}
if ($PSBoundParameters.ContainsKey("MinComponentArea"))
{
	$argList += @("--min-component-area", $MinComponentArea)
}
if ($PSBoundParameters.ContainsKey("PresenceAreaRatioThreshold"))
{
	$argList += @("--presence-area-ratio-threshold", $PresenceAreaRatioThreshold)
}
if ($PSBoundParameters.ContainsKey("PresenceEdgeRatioThreshold"))
{
	$argList += @("--presence-edge-ratio-threshold", $PresenceEdgeRatioThreshold)
}
if ($NoOcr)
{
	$argList += "--no-ocr"
}
if ($PSBoundParameters.ContainsKey("OcrLanguage"))
{
	$argList += @("--ocr-language", $OcrLanguage)
}
if ($PrettyJson)
{
	$argList += "--pretty-json"
}
if ($StrictRequirements)
{
	$argList += "--strict-requirements"
}
if ($NoStrictRequirements)
{
	$argList += "--no-strict-requirements"
}

Write-Host "Running image extraction..."
& $PythonExe @argList
if ($LASTEXITCODE -ne 0)
{
	throw "Image extraction failed (exit $LASTEXITCODE)"
}

Write-Host "Image extraction complete."
Write-Host "JSON: $OutputJson"
Write-Host "CSV:  $OutputCsv"
