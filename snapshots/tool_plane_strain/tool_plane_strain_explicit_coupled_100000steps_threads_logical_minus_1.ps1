$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "tool_plane_strain_explicit_coupled_100000steps.ps1") `
	-ThreadPreset "logical_minus_1" `
	-ResultsDir "results/tool_plane_strain_explicit_coupled_100000steps_threads_logical_minus_1"
