# ClickFromRight: the "Open path report" button sits at the toolbar's right edge
# (move_to_right_edge in library_toolbar.cpp), so the click is measured from the
# client area's right edge. 80 is 2,560 - 2,480 on the user's maximized window.
param([string]$Dir, [string]$Out, [int]$ClickFromRight = 80, [int]$ClickY = 22)
. (Join-Path $PSScriptRoot "..\..\..\tools\capture_window.ps1")
$p = Start-Hydra $Dir 12
$h = $p.MainWindowHandle
$cw = Get-ClientWidth $h
$ClickX = $cw - $ClickFromRight
if ($ClickX -le 0) { $p.Kill(); throw "client area is $cw px wide; ClickFromRight $ClickFromRight misses it" }
$l = Get-ClickParam $ClickX $ClickY
for ($k = 0; $k -lt 10; $k++) { Send-WindowMessage $h 0x0200 0 $l; Start-Sleep -Milliseconds 50 }
Send-WindowMessage $h 0x0201 1 $l
for ($k = 0; $k -lt 4; $k++) { Send-WindowMessage $h 0x0200 1 $l; Start-Sleep -Milliseconds 50 }
Send-WindowMessage $h 0x0202 0 $l
# The report's OS window takes its title from the ImGui name "Path report — Hydra###pathreport",
# so match the title; a tooltip or popup window must not count as the report.
$found = $false
for ($k = 0; $k -lt 30; $k++) {
  Start-Sleep -Seconds 1
  if (@(Get-ProcessWindows $p | Where-Object { (Get-WindowTitle $_) -like "Path report*" }).Count -gt 0) { "report window after $k s"; $found = $true; Start-Sleep -Seconds 3; break }
}
if (-not $found) { $p.Kill(); throw "no 'Path report' window within 30 s of the click at client ($ClickX, $ClickY)" }
$base = $Out -replace '(?i)\.png$', ''
$i = 0
foreach ($w in Get-ProcessWindows $p) { Save-WindowShot $w "$base-$i.png"; $i++ }
Stop-Hydra $p
"exit"
