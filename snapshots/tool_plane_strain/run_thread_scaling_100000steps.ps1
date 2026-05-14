param(
	[switch]$BuildFirst,

	[int[]]$CustomThreadCounts = @()
)

$ErrorActionPreference = "Stop"

$compareScript = Join-Path $PSScriptRoot "run_and_compare_thread_scaling_100000steps.ps1"

& $compareScript -BuildFirst:$BuildFirst -CustomThreadCounts $CustomThreadCounts
