param(
	[string]$BaseDir = ".\results\fe_bc_validate",
	[string]$OutSubdir = "pv_png",
	[string]$ParaViewHome = "",
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

if ($ParaViewHome -and $ParaViewHome.Trim().Length -gt 0) {
	$env:PARAVIEW_HOME = $ParaViewHome
}

foreach ($m in 1..4) {
	$modelDir = Join-Path $BaseDir "model_$m"
	$feVtk = Join-Path $modelDir "fe_tool_000000.vtk"
	$topCsv = Join-Path $modelDir "fe_bc_top_edge.csv"
	$rearCsv = Join-Path $modelDir "fe_bc_rear_edge.csv"
	$outDir = Join-Path $modelDir $OutSubdir

	Write-Host "Generating ParaView PNGs for model_$m -> $outDir"

	.\scripts\run_paraview_batch_vis.ps1 `
		-FeVtk $feVtk `
		-TopCsv $topCsv `
		-RearCsv $rearCsv `
		-OutDir $outDir `
		-ModelLabel "model_$m" `
		-CuttingSpeed_m_min $CuttingSpeed_m_min `
		-Width $Width `
		-Height $Height `
		-NodeGlyphScale $NodeGlyphScale `
		-VelocityGlyphScale $VelocityGlyphScale `
		-SkipEdge:$SkipEdge `
		-SkipVelocity:$SkipVelocity `
		-Verbose:$Verbose
}

