param(
	[Parameter(Mandatory = $true)]
	[string]$PdfPath,
	[string]$OutDir = "results\pdf_assess",
	[string]$Keywords = "meshfree,SPH,thermal,contact,plasticity,refinement,tool,workpiece"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$assessor = Join-Path $repoRoot "scripts\assess_pdf.py"
if (-not (Test-Path $assessor)) { throw "missing script: $assessor" }
if (-not (Test-Path $PdfPath)) { throw "missing PDF: $PdfPath" }

python $assessor --pdf $PdfPath --out-dir $OutDir --keywords $Keywords
