param([string]$Dir, [string]$Out, [int]$Wait = 10, [switch]$Keep)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class W {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
}
"@
[W]::SetProcessDPIAware() | Out-Null
$p = Start-Process -FilePath (Join-Path $Dir "Hydra.exe") -WorkingDirectory $Dir -PassThru
Start-Sleep -Seconds $Wait
$p.Refresh()
$h = $p.MainWindowHandle
[W]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 700
$r = New-Object W+RECT
[W]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.R - $r.L; $hgt = $r.B - $r.T
"rect: $($r.L),$($r.T) ${w}x$hgt"
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$dc = $g.GetHdc(); [W]::PrintWindow($h, $dc, 2) | Out-Null; $g.ReleaseHdc($dc)
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
if (-not $Keep) { $p.CloseMainWindow() | Out-Null; if (-not $p.WaitForExit(10000)) { $p.Kill() } }
"pid: $($p.Id)"
