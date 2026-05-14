param(
	[switch]$BuildFirst,

	[int[]]$CustomThreadCounts = @()
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try
{
	if ($BuildFirst)
	{
		cmake --build build --config Release
		if ($LASTEXITCODE -ne 0)
		{
			throw "Release build failed with exit code $LASTEXITCODE."
		}
	}

	$baseScript = Join-Path $PSScriptRoot "tool_plane_strain_explicit_coupled_100000steps.ps1"
	$summaryDir = "results/thread_scaling_100000steps"
	New-Item -ItemType Directory -Force $summaryDir | Out-Null

	$cases = @(
		[PSCustomObject]@{
			Name = "physical"
			Preset = "physical"
			ThreadCount = 0
			ResultsDir = "results/tool_plane_strain_explicit_coupled_100000steps_threads_physical"
		},
		[PSCustomObject]@{
			Name = "logical_minus_2"
			Preset = "logical_minus_2"
			ThreadCount = 0
			ResultsDir = "results/tool_plane_strain_explicit_coupled_100000steps_threads_logical_minus_2"
		},
		[PSCustomObject]@{
			Name = "logical_minus_1"
			Preset = "logical_minus_1"
			ThreadCount = 0
			ResultsDir = "results/tool_plane_strain_explicit_coupled_100000steps_threads_logical_minus_1"
		},
		[PSCustomObject]@{
			Name = "logical"
			Preset = "logical"
			ThreadCount = 0
			ResultsDir = "results/tool_plane_strain_explicit_coupled_100000steps_threads_logical"
		}
	)

	foreach ($threadCount in $CustomThreadCounts)
	{
		if ($threadCount -lt 1)
		{
			throw "Custom thread counts must be >= 1."
		}
		$cases += [PSCustomObject]@{
			Name = "custom_${threadCount}"
			Preset = "custom"
			ThreadCount = $threadCount
			ResultsDir = "results/tool_plane_strain_explicit_coupled_100000steps_threads_custom_${threadCount}"
		}
	}

	$summary = @()

	foreach ($case in $cases)
	{
		Write-Host ""
		Write-Host "============================================================"
		Write-Host "Running thread setup: $($case.Name)"
		Write-Host "============================================================"

		New-Item -ItemType Directory -Force $case.ResultsDir | Out-Null
		$consoleLog = Join-Path $case.ResultsDir "console.log"
		Remove-Item $consoleLog -ErrorAction SilentlyContinue

		$sw = [System.Diagnostics.Stopwatch]::StartNew()
		if ($case.Preset -eq "custom")
		{
			& $baseScript -ThreadPreset $case.Preset -ThreadCount $case.ThreadCount -ResultsDir $case.ResultsDir 2>&1 | Tee-Object -FilePath $consoleLog
		} else
		{
			& $baseScript -ThreadPreset $case.Preset -ResultsDir $case.ResultsDir 2>&1 | Tee-Object -FilePath $consoleLog
		}
		$sw.Stop()

		$energyPath = Join-Path $case.ResultsDir "cutting_energy.csv"
		$thermalPath = Join-Path $case.ResultsDir "cutting_thermal.csv"
		$metricsPath = Join-Path $case.ResultsDir "cutting_metrics.csv"

		foreach ($path in @($energyPath, $thermalPath, $metricsPath))
		{
			if (-not (Test-Path $path))
			{
				throw "Missing expected output file: $path"
			}
		}

		$energy = Import-Csv $energyPath
		$thermal = Import-Csv $thermalPath
		$metrics = Import-Csv $metricsPath

		$firstEnergy = $energy[0]
		$lastEnergy = $energy[-1]
		$lastThermal = $thermal[-1]
		$lastMetrics = $metrics[-1]

		$runtimeLine = Select-String -Path $consoleLog -Pattern "Runtime:" | Select-Object -Last 1
		$solverRuntimeSeconds = [double]::NaN
		if ($runtimeLine)
		{
			$tokens = $runtimeLine.Line.Trim().Split(" ", [System.StringSplitOptions]::RemoveEmptyEntries)
			if ($tokens.Count -ge 2)
			{
				[double]::TryParse($tokens[-1], [ref]$solverRuntimeSeconds) | Out-Null
			}
		}

		$toolTmaxMax = ($thermal | ForEach-Object { [double]$_.tool_Tmax } | Measure-Object -Maximum).Maximum
		$wpTmaxMax = ($metrics | ForEach-Object { [double]$_.wp_Tmax } | Measure-Object -Maximum).Maximum
		$wpTavgMax = ($metrics | ForEach-Object { [double]$_.wp_Tavg } | Measure-Object -Maximum).Maximum
		$wpEpsplMax = ($metrics | ForEach-Object { [double]$_.wp_epspl_max } | Measure-Object -Maximum).Maximum
		$wpContactPmaxMax = ($metrics | ForEach-Object { [double]$_.wp_contact_pmax } | Measure-Object -Maximum).Maximum
		$maxPredDt = ($energy | ForEach-Object { [double]$_.step_contact_max_pred_dT } | Measure-Object -Maximum).Maximum
		$maxLimiterSuppressed = ($energy | ForEach-Object { [double]$_.step_contact_E_limiter_suppressed } | Measure-Object -Maximum).Maximum

		$finalStep = [int]$lastEnergy.step
		$energyRows = [int]$energy.Count
		$thermalRows = [int]$thermal.Count
		$metricsRows = [int]$metrics.Count
		$cumInterfaceResidual = [double]$lastEnergy.cum_interface_balance_residual
		$cumToolSourceResidual = [double]$lastEnergy.cum_tool_source_residual

		$valid = ($finalStep -eq 100000) -and ($energyRows -eq 100001) -and ($thermalRows -eq 100001) -and ($metricsRows -eq 100001) -and `
		([Math]::Abs($cumInterfaceResidual) -lt 1.0e-8) -and ([Math]::Abs($cumToolSourceResidual) -lt 1.0e-8)

		$summary += [PSCustomObject]@{
			Case = $case.Name
			ThreadPreset = $case.Preset
			RequestedThreadCount = $case.ThreadCount
			ResultsDir = $case.ResultsDir
			WallSeconds = [Math]::Round($sw.Elapsed.TotalSeconds, 6)
			SolverRuntimeSeconds = $solverRuntimeSeconds
			EnergyRows = $energyRows
			ThermalRows = $thermalRows
			MetricsRows = $metricsRows
			FinalStep = $finalStep
			FinalTime = [double]$lastEnergy.time
			ToolDeltaE = ([double]$lastEnergy.tool_internal_E - [double]$firstEnergy.tool_internal_E)
			WpDeltaE = ([double]$lastEnergy.wp_internal_E - [double]$firstEnergy.wp_internal_E)
			CumFrictionE = [double]$lastEnergy.cum_contact_E_fric_scaled
			CumConductionE = [double]$lastEnergy.cum_contact_E_cond_scaled
			CumToolContactE = [double]$lastEnergy.cum_contact_E_tool
			CumWpContactE = [double]$lastEnergy.cum_contact_E_workpiece
			CumToolConvectionE = [double]$lastEnergy.cum_tool_E_convection
			CumInterfaceResidual = $cumInterfaceResidual
			CumToolSourceResidual = $cumToolSourceResidual
			FinalToolTmax = [double]$lastThermal.tool_Tmax
			MaxToolTmax = [double]$toolTmaxMax
			FinalWpTmax = [double]$lastMetrics.wp_Tmax
			MaxWpTmax = [double]$wpTmaxMax
			MaxWpTavg = [double]$wpTavgMax
			MaxWpEpspl = [double]$wpEpsplMax
			MaxWpContactPmax = [double]$wpContactPmaxMax
			MaxStepContactPredDt = [double]$maxPredDt
			MaxStepLimiterSuppressed = [double]$maxLimiterSuppressed
			ValidForTimingDecision = [bool]$valid
			ConsoleLog = $consoleLog
		}
	}

	$physical = $summary | Where-Object { $_.Case -eq "physical" } | Select-Object -First 1
	if ($physical)
	{
		foreach ($row in $summary)
		{
			$row | Add-Member -NotePropertyName SpeedupVsPhysicalWall -NotePropertyValue ([Math]::Round($physical.WallSeconds / $row.WallSeconds, 6)) -Force
			if ([double]::IsNaN($physical.SolverRuntimeSeconds) -or [double]::IsNaN($row.SolverRuntimeSeconds))
			{
				$row | Add-Member -NotePropertyName SpeedupVsPhysicalSolver -NotePropertyValue ([double]::NaN) -Force
			} else
			{
				$row | Add-Member -NotePropertyName SpeedupVsPhysicalSolver -NotePropertyValue ([Math]::Round($physical.SolverRuntimeSeconds / $row.SolverRuntimeSeconds, 6)) -Force
			}
		}
	}

	$summaryPath = Join-Path $summaryDir "thread_scaling_summary.csv"
	$summary | Export-Csv $summaryPath -NoTypeInformation

	$validRows = $summary | Where-Object { $_.ValidForTimingDecision } | Sort-Object WallSeconds
	$recommendationPath = Join-Path $summaryDir "thread_scaling_recommendation.txt"
	if ($validRows.Count -gt 0)
	{
		$best = $validRows | Select-Object -First 1
		@(
			"Recommended thread setup for DOE: $($best.Case)",
			"WallSeconds: $($best.WallSeconds)",
			"SolverRuntimeSeconds: $($best.SolverRuntimeSeconds)",
			"SpeedupVsPhysicalWall: $($best.SpeedupVsPhysicalWall)",
			"ResultsDir: $($best.ResultsDir)",
			"",
			"Decision rule: fastest valid run with completed step/row counts and near-zero energy residuals.",
			"Check thread_scaling_summary.csv for detailed energy, thermal, and mechanical consistency metrics."
		) | Set-Content $recommendationPath
	} else
	{
		"No valid timing recommendation: one or more required completion/energy-residual checks failed." | Set-Content $recommendationPath
	}

	Write-Host ""
	Write-Host "Thread scaling summary: $summaryPath"
	Write-Host "Recommendation: $recommendationPath"
	$summary | Sort-Object WallSeconds | Format-Table Case, WallSeconds, SolverRuntimeSeconds, SpeedupVsPhysicalWall, FinalStep, EnergyRows, ValidForTimingDecision -AutoSize
} finally
{
	Pop-Location
}
