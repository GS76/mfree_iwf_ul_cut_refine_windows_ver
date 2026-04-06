$ErrorActionPreference = "Stop"

$files = Get-ChildItem -Path ".\results" -Filter "fe_tool_*.vtk" | Sort-Object Name
if ($files.Count -lt 2) {
  Write-Error "Need at least two results/fe_tool_*.vtk files"
}

function Get-FirstPointX([string]$path) {
  $lines = Get-Content -Path $path -TotalCount 12
  foreach ($l in $lines) {
    if ($l -match "^\s*POINTS\s+\d+\s+") { continue }
    if ($l -match "^\s*#") { continue }
    if ($l -match "^\s*$") { continue }
    if ($l -match "^\s*DATASET\s+") { continue }
    if ($l -match "^\s*ASCII\s*$") { continue }
    if ($l -match "^\s*mfree") { continue }
    $parts = $l.Trim() -split "\s+"
    if ($parts.Length -ge 3) {
      return [double]$parts[0]
    }
  }
  throw "Could not parse first POINTS line from $path"
}

$x0 = Get-FirstPointX $files[0].FullName
$x1 = Get-FirstPointX $files[-1].FullName
$dx = $x1 - $x0

Write-Host ("First frame : {0}  x0 = {1:E12}" -f $files[0].Name, $x0)
Write-Host ("Last frame  : {0}  x1 = {1:E12}" -f $files[-1].Name, $x1)
Write-Host ("Delta x     : dx = {0:E12} m" -f $dx)
Write-Host ("Delta x (um): {0:F6} um" -f ($dx * 1e6))

