param([string]$Dir, [string]$Prefix, [string[]]$Steps = @("start","min","restore","max"))
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class W2 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
"@
[W2]::SetProcessDPIAware() | Out-Null
function Shot($h, $out) {
  $r = New-Object W2+RECT
  [W2]::GetWindowRect($h, [ref]$r) | Out-Null
  $w = $r.R - $r.L; $hgt = $r.B - $r.T
  if ($w -le 0 -or $hgt -le 0) { "skip $out (rect ${w}x$hgt)"; return }
  $bmp = New-Object System.Drawing.Bitmap $w, $hgt
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $dc = $g.GetHdc(); [W2]::PrintWindow($h, $dc, 2) | Out-Null; $g.ReleaseHdc($dc)
  $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
  "$out rect $($r.L),$($r.T) ${w}x$hgt"
}
$p = Start-Process -FilePath (Join-Path $Dir "Hydra.exe") -WorkingDirectory $Dir -PassThru
Start-Sleep -Seconds 12
$p.Refresh(); $h = $p.MainWindowHandle
$codes = @{ min = 6; restore = 9; max = 3 }
function Post($msg, $w, $l) { [W2]::PostMessage($h, $msg, [IntPtr]$w, [IntPtr]$l) | Out-Null }
foreach ($s in $Steps) {
  if ($s -like "click*") {
    $xy = $s.Substring(5).Split("x"); $l = ([int]$xy[1] -shl 16) -bor [int]$xy[0]
    Post 0x0200 0 $l; Start-Sleep -Milliseconds 300; Post 0x0201 1 $l; Start-Sleep -Milliseconds 150; Post 0x0202 0 $l
    Start-Sleep -Seconds 6
  } elseif ($s -eq "esc") {
    Post 0x0100 0x1B 0x00010001; Start-Sleep -Milliseconds 150; Post 0x0101 0x1B 0xC0010001; Start-Sleep -Seconds 3
  } elseif ($s -ne "start") { [W2]::ShowWindow($h, $codes[$s]) | Out-Null; Start-Sleep -Seconds 3 }
  Shot $h "$Prefix-$s.png"
}
$p.CloseMainWindow() | Out-Null; if (-not $p.WaitForExit(10000)) { $p.Kill() }
"exit"
