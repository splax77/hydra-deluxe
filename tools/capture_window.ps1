# Shared helpers for scripted captures of a real Hydra window. Dot-source it:
#
#   . (Join-Path $PSScriptRoot "..\..\..\tools\capture_window.ps1")
#
# The capture scripts in docs/handoffs/2026-10-10-maximized-report-stretch/
# (shot.ps1, steps.ps1, report_shot.ps1) are its callers. Start-Hydra launches
# a scratch copy of Hydra.exe and fails loudly when no main window shows up;
# Save-WindowShot draws one window into a PNG with PrintWindow; the rest are
# thin wrappers over user32 calls, all on window handles of that process.
#
# A PowerShell host keeps every Add-Type type for its whole life, and a changed
# C# body under a name it already loaded never takes effect: calls to the new
# members fail or do nothing. So the type name carries a version. Bump it
# whenever the C# block below changes.

Add-Type -AssemblyName System.Drawing
if (-not ('HydraCaptureV1' -as [type])) {
  Add-Type @"
using System; using System.Runtime.InteropServices; using System.Collections.Generic; using System.Text;
public static class HydraCaptureV1 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  public static List<IntPtr> Of(uint pid) {
    var list = new List<IntPtr>();
    EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p == pid && IsWindowVisible(h)) list.Add(h); return true; }, IntPtr.Zero);
    return list;
  }
  public static string Title(IntPtr h) { var s = new StringBuilder(256); GetWindowText(h, s, 256); return s.ToString(); }
}
"@
}
# Physical pixels everywhere: window rects, client coordinates and captures.
[HydraCaptureV1]::SetProcessDPIAware() | Out-Null

# Launch <Dir>\Hydra.exe with <Dir> as its working folder (it reads the ini
# files and db there), wait $Wait seconds, and return the process. Throws, after
# killing it, when there is still no main window (still starting, or a startup
# box is up).
function Start-Hydra([string]$Dir, [int]$Wait = 12) {
  $p = Start-Process -FilePath (Join-Path $Dir "Hydra.exe") -WorkingDirectory $Dir -PassThru
  Start-Sleep -Seconds $Wait
  $p.Refresh()
  if ($p.MainWindowHandle -eq [IntPtr]::Zero) { $p.Kill(); throw "Hydra has no main window after $Wait s (still starting, or a startup box is up)" }
  $p
}

function Stop-Hydra($p) {
  $p.CloseMainWindow() | Out-Null
  if (-not $p.WaitForExit(10000)) { $p.Kill() }
}

# Every visible top-level window of the process, main window included.
function Get-ProcessWindows($p) { [HydraCaptureV1]::Of([uint32]$p.Id) }

function Get-WindowTitle([IntPtr]$h) { [HydraCaptureV1]::Title($h) }

function Get-ClientWidth([IntPtr]$h) {
  $c = New-Object HydraCaptureV1+RECT; [HydraCaptureV1]::GetClientRect($h, [ref]$c) | Out-Null
  $c.R
}

function Send-WindowMessage([IntPtr]$h, [uint32]$msg, $w, $l) {
  [HydraCaptureV1]::PostMessage($h, $msg, [IntPtr]$w, [IntPtr]$l) | Out-Null
}

# lParam for a mouse message at client (x, y).
function Get-ClickParam([int]$x, [int]$y) { ($y -shl 16) -bor $x }

# PrintWindow flag 2 (PW_RENDERFULLCONTENT) so the GPU-drawn frame is captured.
# Skips a window with an empty rect rather than throwing. Returns one line
# naming the file and the window rect.
function Save-WindowShot([IntPtr]$h, [string]$out) {
  $r = New-Object HydraCaptureV1+RECT; [HydraCaptureV1]::GetWindowRect($h, [ref]$r) | Out-Null
  $w = $r.R - $r.L; $hgt = $r.B - $r.T
  if ($w -le 0 -or $hgt -le 0) { "skip $out (rect ${w}x$hgt)"; return }
  $bmp = New-Object System.Drawing.Bitmap $w, $hgt
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  try {
    $dc = $g.GetHdc(); [HydraCaptureV1]::PrintWindow($h, $dc, 2) | Out-Null; $g.ReleaseHdc($dc)
    $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
  } finally { $g.Dispose(); $bmp.Dispose() }
  "$out rect $($r.L),$($r.T) ${w}x$hgt"
}
