param(
	[Parameter(Mandatory = $true)]
	[ValidateRange(1, 4096)]
	[int]$ThreadCount
)

$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "tool_plane_strain_explicit_coupled_100000steps.ps1") `
	-ThreadPreset "custom" `
	-ThreadCount $ThreadCount `
	-ResultsDir "results/tool_plane_strain_explicit_coupled_100000steps_threads_custom_${ThreadCount}"
