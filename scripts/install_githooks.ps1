param(
	[switch]$Force = $false
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path ".git")) { throw "run from repo root (missing .git)" }

$srcDir = Join-Path $PSScriptRoot "githooks"
$dstDir = Join-Path (Resolve-Path ".git").Path "hooks"

if (-not (Test-Path $srcDir)) { throw "missing hooks template dir: $srcDir" }
New-Item -ItemType Directory -Force -Path $dstDir | Out-Null

$hooks = @("pre-commit", "commit-msg")
foreach ($h in $hooks) {
	$src = Join-Path $srcDir $h
	$dst = Join-Path $dstDir $h
	if (-not (Test-Path $src)) { throw "missing hook template: $src" }
	if ((Test-Path $dst) -and -not $Force) { throw "hook already exists: $dst (use -Force to overwrite)" }
	Copy-Item -Force $src $dst
}

Write-Host "installed hooks to $dstDir"

