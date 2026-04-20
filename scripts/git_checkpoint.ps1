param(
	[string]$Milestone,
	[string]$Message = "",
	[string]$Tag = "",
	[switch]$UseAddPatch = $false,
	[switch]$AllowUntracked = $false,
	[switch]$DryRun = $false,
	[switch]$NoTag = $false
)

$ErrorActionPreference = "Stop"

function Require-RepoRoot {
	if (-not (Test-Path ".git")) { throw "run from repo root (missing .git)" }
}

Require-RepoRoot

$env:GIT_PAGER = "cat"
$env:PAGER = "cat"

git --no-pager status

if (-not $UseAddPatch -and -not $AllowUntracked) {
	$untracked = git status --porcelain | Where-Object { $_ -match '^\?\?' }
	if ($untracked) {
		$u = ($untracked -join "`n")
		throw "untracked files exist; use -UseAddPatch (recommended) or pass -AllowUntracked to proceed.`n$u"
	}
}

git --no-pager diff --stat

$diffPath = Join-Path (Get-Location) "checkpoint.diff"
git --no-pager diff --no-color | Set-Content -Path $diffPath -Encoding UTF8
Write-Host "wrote diff to $diffPath"

if (-not $Milestone -or $Milestone.Trim().Length -eq 0) {
	throw "Milestone is required (e.g., fe-bc-validate, pv-batch, fe-tool-thermal)"
}

if (-not $Message -or $Message.Trim().Length -eq 0) {
	$Message = "${Milestone}: checkpoint"
}

if ($DryRun) {
	Write-Host "dry-run: skipping commit/tag"
	exit 0
}

if ($UseAddPatch) {
	git add -p
	git commit -m $Message
} else {
	git commit -am $Message
}

if (-not $NoTag) {
	if (-not $Tag -or $Tag.Trim().Length -eq 0) {
		$dt = Get-Date -Format "yyyyMMdd"
		$Tag = "$Milestone-$dt-v1"
	}
	git tag $Tag
}
