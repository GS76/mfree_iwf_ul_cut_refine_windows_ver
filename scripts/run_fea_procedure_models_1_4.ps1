param(
	[string]$ResultsRoot = "results\fea_procedure",
	[double]$CuttingSpeed_m_min = 100.0,
	[double]$FeedPerRev_mm = 0.2,
	[double]$BaseTargetFeed_mm = 0.1,
	[double]$WorkpieceThickness_mm = 0.5,
	[int]   $MaxSteps = 2000,
	[int]   $OutputSteps = 20,
	[int]   $RenderFrames = 10,
	[string]$RenderField = "displacement",
	[string]$ParaViewHome = "",
	[switch]$GeneratePNGs = $true,
	[switch]$GenerateReport = $true,
	[switch]$GeneratePDF = $true
)

$ErrorActionPreference = "Stop"

$exe = ".\build\Release\mfree_iwf.exe"
$mesh = ".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"

if ($ParaViewHome -and $ParaViewHome.Trim().Length -gt 0)
{
	$env:PARAVIEW_HOME = $ParaViewHome
}

$env:MFREE_FE_TOOL_MSH             = $mesh
$env:MFREE_NO_RIGID_TOOL           = "1"
$env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
$env:MFREE_CONTACT_MU              = "0.35"

$env:MFREE_CUTTING_SPEED_M_MIN    = "$CuttingSpeed_m_min"
$env:MFREE_NBOX                   = "61"
$env:MFREE_FEED_PER_REV_MM        = "$FeedPerRev_mm"
$env:MFREE_BASE_TARGET_FEED_MM    = "$BaseTargetFeed_mm"
$env:MFREE_WORKPIECE_THICKNESS_MM = "$WorkpieceThickness_mm"

$env:MFREE_DEFORMABLE_FE_TOOL = "1"
$env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT = "1"
$env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "25"

$env:MFREE_LOG_THERMAL = "1"
$env:MFREE_LOG_METRICS = "1"
$env:MFREE_LOG_VTK_WORKPIECE = "1"
$env:MFREE_LOG_VTK_TOOL = "0"
$env:MFREE_LOG_VTK_FE_TOOL = "1"

$env:MFREE_MAX_STEPS = "$MaxSteps"
$env:MFREE_NUM_PRINT = "$OutputSteps"
$freq = 1
if ($OutputSteps -gt 0)
{ $freq = [Math]::Max(1, [int]($MaxSteps / $OutputSteps))
}
$env:MFREE_OUTPUT_FREQ = "$freq"

$modelSummaries = @()

function Select-FrameIndices([string]$ModelDir, [int]$Count)
{
	$files = Get-ChildItem -Path $ModelDir -Filter "out_*.vtk" -ErrorAction SilentlyContinue | Sort-Object Name
	$idx = @()
	foreach ($f in $files)
	{
		if ($f.BaseName -match '^out_(\d+)$')
		{ $idx += [int]$Matches[1]
  }
	}
	$idx = $idx | Sort-Object -Unique
	if ($idx.Count -eq 0)
	{ return @()
 }
	if ($Count -le 0)
	{ return $idx
 }
	if ($idx.Count -le $Count)
	{ return $idx
 }
	$out = @()
	for ($i = 0; $i -lt $Count; $i++)
	{
		$t = if ($Count -eq 1)
		{ 0.0
  } else
		{ $i / ($Count - 1.0)
  }
		$j = [int][Math]::Round($t * ($idx.Count - 1))
		$out += $idx[$j]
	}
	return ($out | Sort-Object -Unique)
}

$modelDir = Join-Path $ResultsRoot "model_3"
New-Item -ItemType Directory -Force -Path $modelDir | Out-Null

$env:MFREE_RESULTS_DIR   = $modelDir
$env:MFREE_CLEAN_RESULTS = "1"

$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_FE_BC_VALIDATE  = "1"
$env:MFREE_FE_BC_TOP_TAG   = "110"
$env:MFREE_FE_BC_REAR_TAG  = "114"
$env:MFREE_FE_BC_AMBIENT_C = "25"
$env:MFREE_FE_BC_ANCHOR_UX = "1"
& $exe -m 3

if ($GeneratePNGs)
{
	.\scripts\run_paraview_batch_vis.ps1 `
		-FeVtk  (Join-Path $modelDir "fe_tool_000000.vtk") `
		-TopCsv (Join-Path $modelDir "fe_bc_top_edge.csv") `
		-RearCsv (Join-Path $modelDir "fe_bc_rear_edge.csv") `
		-OutDir (Join-Path $modelDir "pv_bc_png") `
		-ModelLabel "model_3" `
		-CuttingSpeed_m_min $CuttingSpeed_m_min
}

$env:MFREE_PREPROCESS_ONLY = "0"
& $exe -m 3

if ($GeneratePNGs)
{
	$pvpython = Join-Path $env:PARAVIEW_HOME "bin\pvpython.exe"
	if (Test-Path $pvpython)
	{
		$frames = Select-FrameIndices -ModelDir $modelDir -Count $RenderFrames
		$steps = ($frames | ForEach-Object { "$_" }) -join ","
		if (-not $steps)
		{ $steps = "0"
  }
		& $pvpython .\scripts\paraview_batch_advancement.py `
			--wp-vtk-pattern (Join-Path $modelDir "out_%06d.vtk") `
			--fe-vtk-pattern (Join-Path $modelDir "fe_tool_%06d.vtk") `
			--steps $steps `
			--field $RenderField `
			--out-dir (Join-Path $modelDir "pv_adv_png") `
			--model-label "model_3"
	}
}

$bcReportPath   = Join-Path $modelDir "fe_bc_report.json"
$valSummaryPath = Join-Path $modelDir "validation_summary.json"
$metricsPath    = Join-Path $modelDir "cutting_metrics.csv"
$thermalPath    = Join-Path $modelDir "cutting_thermal.csv"

$bcReport = $null
if (Test-Path $bcReportPath)
{ $bcReport   = Get-Content -Raw $bcReportPath   | ConvertFrom-Json
}
$valSummary = $null
if (Test-Path $valSummaryPath)
{ $valSummary = Get-Content -Raw $valSummaryPath | ConvertFrom-Json
}
$lastMetrics = $null
if (Test-Path $metricsPath)
{ $lastMetrics = (Import-Csv $metricsPath | Select-Object -Last 1)
}
$lastThermal = $null
if (Test-Path $thermalPath)
{ $lastThermal = (Import-Csv $thermalPath | Select-Object -Last 1)
}

$modelSummaries += [pscustomobject]@{
	model              = 3
	results_dir        = $modelDir
	bc_top_nodes       = if ($bcReport)
	{ $bcReport.top_nodes
 } else
	{ $null
 }
	bc_rear_nodes      = if ($bcReport)
	{ $bcReport.rear_nodes
 } else
	{ $null
 }
	bc_temp_err_top_K  = if ($bcReport)
	{ $bcReport.max_abs_temp_err_K.top
 } else
	{ $null
 }
	bc_temp_err_rear_K = if ($bcReport)
	{ $bcReport.max_abs_temp_err_K.rear
 } else
	{ $null
 }
	wp_umax_m          = if ($lastMetrics)
	{ [double]$lastMetrics.wp_umax
 } else
	{ $null
 }
	wp_svm_max         = if ($lastMetrics)
	{ [double]$lastMetrics.wp_svm_max
 } else
	{ $null
 }
	wp_epspl_max       = if ($lastMetrics)
	{ [double]$lastMetrics.wp_epspl_max
 } else
	{ $null
 }
	wp_contact_pmax    = if ($lastMetrics)
	{ [double]$lastMetrics.wp_contact_pmax
 } else
	{ $null
 }
	P_cond_W           = if ($lastThermal)
	{ [double]$lastThermal.P_cond_W
 } else
	{ $null
 }
	P_fric_W           = if ($lastThermal)
	{ [double]$lastThermal.P_fric_W
 } else
	{ $null
 }
	tool_vel_x         = if ($lastThermal)
	{ [double]$lastThermal.tool_vel_x
 } else
	{ $null
 }
	max_disp_validation = if ($valSummary -and $valSummary.workpiece)
	{ $valSummary.workpiece.max_displacement
 } else
	{ $null
 }
}

if ($GenerateReport)
{
	New-Item -ItemType Directory -Force -Path $ResultsRoot | Out-Null
	$mdPath = Join-Path $ResultsRoot "report.md"
	$now = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
	function Fmt([object]$v)
	{
		if ($null -eq $v)
		{ return "NA"
  }
		return ("{0:E3}" -f ([double]$v))
	}
	function FmtInt([object]$v)
	{
		if ($null -eq $v)
		{ return "NA"
  }
		return ("{0}" -f ([int]$v))
	}
	$lines = @()
	$lines += "# FE Advancement Procedure Report (Model 3)"
	$lines += ""
	$lines += "- Generated: $now"
	$lines += "- Cutting speed: $CuttingSpeed_m_min m/min"
	$lines += "- Feed per rev (clearance target): $FeedPerRev_mm mm"
	$lines += "- Base target feed: $BaseTargetFeed_mm mm"
	$lines += "- Workpiece thickness: $WorkpieceThickness_mm mm"
	$lines += "- Max steps: $MaxSteps"
	$lines += "- Output frames requested: $OutputSteps"
	$lines += "- Render field: $RenderField"
	$lines += ""
	$lines += "## Summary Table"
	$lines += ""
	$lines += "| Model | Top nodes | Rear nodes | max|T-Tamb| top (K) | max|T-Tamb| rear (K) | tool_vel_x (m/s) | wp_umax (m) | wp_svm_max | wp_epspl_max | wp_contact_pmax | P_cond (W) |"
	$lines += "|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|"
	foreach ($r in $modelSummaries)
	{
		$lines += ("| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} | {8} | {9} | {10} |" -f `
				$r.model,
			(FmtInt $r.bc_top_nodes),
			(FmtInt $r.bc_rear_nodes),
			(Fmt $r.bc_temp_err_top_K),
			(Fmt $r.bc_temp_err_rear_K),
			(Fmt $r.tool_vel_x),
			(Fmt $r.wp_umax_m),
			(Fmt $r.wp_svm_max),
			(Fmt $r.wp_epspl_max),
			(Fmt $r.wp_contact_pmax),
			(Fmt $r.P_cond_W))
	}
	$lines += ""
	foreach ($r in $modelSummaries)
	{
		$rel = Resolve-Path $r.results_dir
		$lines += "## Model 3"
		$lines += ""
		$lines += "- Results: $rel"
		$lines += '- BC report: `fe_bc_report.json`, `fe_bc_top_edge.csv`, `fe_bc_rear_edge.csv`'
		$lines += '- Simulation logs: `cutting_metrics.csv`, `cutting_thermal.csv`'
		$lines += '- PNGs: `pv_bc_png/` and `pv_adv_png/`'
		$lines += ""
		$lines += "### BC Visualizations"
		$lines += ""
		$lines += "![](./model_3/pv_bc_png/edge_nodes.png)"
		$lines += ""
		$lines += "![](./model_3/pv_bc_png/velocity_glyphs.png)"
		$lines += ""
		$lines += "### Advancement Frames ($RenderField)"
		$lines += ""
		$advDir = Join-Path $r.results_dir "pv_adv_png"
		$pngs = Get-ChildItem -Path $advDir -Filter ("adv_{0}_*.png" -f $RenderField) -ErrorAction SilentlyContinue | Sort-Object Name
		foreach ($p in $pngs)
		{
			$lines += "![](./model_3/pv_adv_png/$($p.Name))"
			$lines += ""
		}
	}
	$pdfStatus = "skipped"
	$pdfError = ""
	Set-Content -Path $mdPath -Value $lines -Encoding UTF8

	if ($GeneratePDF)
	{
		$pandoc = Get-Command pandoc -ErrorAction SilentlyContinue
		if ($pandoc)
		{
			try
			{
				Push-Location $ResultsRoot
				& $pandoc.Source "report.md" -o "report.pdf" 2>$null | Out-Null
				$ec = $LASTEXITCODE
				Pop-Location
				if ($ec -eq 0 -and (Test-Path (Join-Path $ResultsRoot "report.pdf")))
				{
					$pdfStatus = "generated"
				} else
				{
					$pdfStatus = "failed"
					$pdfError = "pandoc exit code $ec"
				}
			} catch
			{
				try
				{ Pop-Location
    } catch
				{
    }
				$pdfStatus = "failed"
				$pdfError = $_.Exception.Message
			}
			$global:LASTEXITCODE = 0
		} else
		{
			$pdfStatus = "skipped"
			$pdfError = "pandoc not found"
		}
	}

	$note = @()
	$note += ""
	$note += "## Report Artifacts"
	$note += ""
	$note += "- Markdown: report.md"
	$note += "- PDF: $pdfStatus"
	if ($pdfError)
	{ $note += "- PDF note: $pdfError"
 }
	Add-Content -Path $mdPath -Value $note -Encoding UTF8
}

$global:LASTEXITCODE = 0
