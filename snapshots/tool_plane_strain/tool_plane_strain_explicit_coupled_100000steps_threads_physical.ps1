$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "tool_plane_strain_explicit_coupled_100000steps.ps1") `
	-ThreadPreset "physical" `
	-ResultsDir "results/tool_plane_strain_explicit_coupled_100000steps_threads_physical"
