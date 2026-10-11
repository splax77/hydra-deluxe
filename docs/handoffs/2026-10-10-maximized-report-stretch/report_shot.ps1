param([string]$Dir, [string]$Out, [int]$ClickX = 2480, [int]$ClickY = 22)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices; using System.Collections.Generic;
public static class W3 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  public static List<IntPtr> Of(uint pid) {
    var list = new List<IntPtr>();
    EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p == pid && IsWindowVisible(h)) list.Add(h); return true; }, IntPtr.Zero);
    return list;
  }
}
"@
[W3]::SetProcessDPIAware() | Out-Null
function Shot($h, $out) {
  $r = New-Object W3+RECT; [W3]::GetWindowRect($h, [ref]$r) | Out-Null
  $w = $r.R - $r.L; $hgt = $r.B - $r.T
  $bmp = New-Object System.Drawing.Bitmap $w, $hgt
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $dc = $g.GetHdc(); [W3]::PrintWindow($h, $dc, 2) | Out-Null; $g.ReleaseHdc($dc)
  $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
  "$out rect $($r.L),$($r.T) ${w}x$hgt"
}
$p = Start-Process -FilePath (Join-Path $Dir "Hydra.exe") -WorkingDirectory $Dir -PassThru
Start-Sleep -Seconds 12
$p.Refresh(); $h = $p.MainWindowHandle
$l = ($ClickY -shl 16) -bor $ClickX
for ($k = 0; $k -lt 10; $k++) { [W3]::PostMessage($h, 0x0200, [IntPtr]0, [IntPtr]$l) | Out-Null; Start-Sleep -Milliseconds 50 }
[W3]::PostMessage($h, 0x0201, [IntPtr]1, [IntPtr]$l) | Out-Null
for ($k = 0; $k -lt 4; $k++) { [W3]::PostMessage($h, 0x0200, [IntPtr]1, [IntPtr]$l) | Out-Null; Start-Sleep -Milliseconds 50 }
[W3]::PostMessage($h, 0x0202, [IntPtr]0, [IntPtr]$l) | Out-Null
for ($k = 0; $k -lt 30; $k++) { Start-Sleep -Seconds 1; if (([W3]::Of([uint32]$p.Id)).Count -gt 1) { "report window after $k s"; Start-Sleep -Seconds 3; break } }
$i = 0
foreach ($w in [W3]::Of([uint32]$p.Id)) { Shot $w ($Out -replace '\.png$', "-$i.png"); $i++ }
$p.CloseMainWindow() | Out-Null; if (-not $p.WaitForExit(10000)) { $p.Kill() }
"exit"
