param([string]$Dir, [string]$Out, [int]$Wait = 10, [switch]$Keep)
. (Join-Path $PSScriptRoot "..\..\..\tools\capture_window.ps1")
$p = Start-Hydra $Dir $Wait
$h = $p.MainWindowHandle
Set-WindowForeground $h
Start-Sleep -Milliseconds 700
Save-WindowShot $h $Out
if (-not $Keep) { Stop-Hydra $p }
"pid: $($p.Id)"
