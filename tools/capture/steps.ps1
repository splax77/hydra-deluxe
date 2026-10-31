# Runs Hydra's main window through a list of steps and captures it after each,
# as <Prefix>-<step>.png. Setup and -Dir: see tools/capture_window.ps1.
param([string]$Dir, [string]$Prefix, [string[]]$Steps = @("start","min","restore","max"))
. (Join-Path $PSScriptRoot "..\capture_window.ps1")
$codes = @{ min = 6; restore = 9; max = 3 }
# An unknown step would make ShowWindow's code null, which is 0 = SW_HIDE.
foreach ($s in $Steps) { if ($s -ne "start" -and $s -ne "esc" -and $s -notmatch '^click\d+x\d+$' -and -not $codes.ContainsKey($s)) { throw "unknown step '$s' (use start, min, restore, max, esc, click<x>x<y>)" } }
$p = Start-Hydra $Dir 12
$h = $p.MainWindowHandle
foreach ($s in $Steps) {
  if ($s -like "click*") {
    $xy = $s.Substring(5).Split("x"); $l = Get-ClickParam ([int]$xy[0]) ([int]$xy[1])
    Send-WindowMessage $h 0x0200 0 $l; Start-Sleep -Milliseconds 300; Send-WindowMessage $h 0x0201 1 $l; Start-Sleep -Milliseconds 150; Send-WindowMessage $h 0x0202 0 $l
    Start-Sleep -Seconds 6
  } elseif ($s -eq "esc") {
    Send-WindowMessage $h 0x0100 0x1B 0x00010001; Start-Sleep -Milliseconds 150; Send-WindowMessage $h 0x0101 0x1B 0xC0010001; Start-Sleep -Seconds 3
  } elseif ($s -ne "start") { Set-WindowShowState $h $codes[$s]; Start-Sleep -Seconds 3 }
  Save-WindowShot $h "$Prefix-$s.png"
}
Stop-Hydra $p
"exit"
