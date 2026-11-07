<#
.SYNOPSIS
Start or stop the ETW trace that runs beside a recorded Clone Hero run.

.DESCRIPTION
ETW is Windows' own event log of who held the CPU and the disk. WPR is the
recorder that writes it to an .etl file, and xperf turns that file into text.

-Start starts the recording. Nothing reaches the .etl until -Stop, so -Start
takes the path only to refuse a bad one before the trace begins. -Stop ends
the recording, writes the .etl, and dumps it to a text file beside it, named
like the .etl but ending in .csv.

Each mode prints the exact command it runs and that command's exit code, and
-Stop then prints the text file's path and size. The script ends with exit
code 0 only when every command succeeded. It starts no game. Where the tools
live, and which shell they need, is in docs/agents/this-machine.md.

.EXAMPLE
pwsh -NoProfile -File tools/ch_mcp/etw/record.ps1 -Start C:\runs\run-a.etl

.EXAMPLE
pwsh -NoProfile -File tools/ch_mcp/etw/record.ps1 -Stop C:\runs\run-a.etl
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Start')] [string] $Start,
    [Parameter(Mandatory = $true, ParameterSetName = 'Stop')] [string] $Stop
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2.0

# Full paths, because the machine has two copies of wpr.exe.
$Wpr = Join-Path $env:SystemRoot 'System32\wpr.exe'
$Xperf = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Windows Performance Toolkit\xperf.exe'

$StartArguments = '-start CPU -start DiskIO -start FileIO -start GPU -start Audio -filemode'

# The longest this script waits, in seconds. The two commands of a -Stop share
# it. The run timing plan's live step LS-T1 sets the number.
$CapSeconds = 120

$Clock = [System.Diagnostics.Stopwatch]::StartNew()

function Exit-Refused([string] $Reason) {
    Write-Host "record.ps1: $Reason"
    exit 1
}

# Runs one command with this window's output, and waits no longer than what is
# left of the cap. Returns its exit code, or $null when the cap stopped it.
# The arguments stay one string, so the printed line is exactly what runs.
function Invoke-Capped([string] $Exe, [string] $Arguments) {
    $leftMs = [math]::Max(0, [int]($CapSeconds * 1000 - $Clock.ElapsedMilliseconds))
    Write-Host "command: & `"$Exe`" $Arguments"
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = $Exe
    $info.Arguments = $Arguments
    $info.UseShellExecute = $false
    $process = [System.Diagnostics.Process]::Start($info)
    if (-not $process.WaitForExit($leftMs)) {
        try {
            $process.Kill()
        } catch {
            Write-Host "record.ps1: could not stop the command: $($_.Exception.Message)"
        }
        Write-Host "exit code: none, stopped at the cap of $CapSeconds s"
        return $null
    }
    $code = $process.ExitCode
    Write-Host ('exit code: {0} (0x{0:X8})' -f $code)
    return $code
}

# Ends the script when a command failed or hit the cap; returns when it passed.
function Assert-Passed($Code, [string] $AtCap) {
    if ($null -eq $Code) {
        Exit-Refused $AtCap
    }
    if ($Code -ne 0) {
        exit $Code
    }
}

$given = if ($PSCmdlet.ParameterSetName -eq 'Start') { $Start } else { $Stop }
$etl = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($given)
$text = [System.IO.Path]::ChangeExtension($etl, '.csv')

if (-not (Test-Path -LiteralPath $Wpr -PathType Leaf)) {
    Exit-Refused "wpr.exe is not at $Wpr"
}
if (-not (Test-Path -LiteralPath $Xperf -PathType Leaf)) {
    Exit-Refused "xperf.exe is not at $Xperf"
}
# wpr stops to ask about any other extension, which nobody may be there to see.
if ([System.IO.Path]::GetExtension($etl) -ne '.etl') {
    Exit-Refused "the trace path must end in .etl: $etl"
}
$folder = Split-Path -Parent $etl
if (-not (Test-Path -LiteralPath $folder -PathType Container)) {
    Exit-Refused "the trace's folder does not exist: $folder"
}
foreach ($path in @($etl, $text)) {
    if (Test-Path -LiteralPath $path) {
        Exit-Refused "this would overwrite an earlier capture, so nothing ran: $path"
    }
}

$wprCapLine = "wpr did not finish. Read 'wpr -status'; 'wpr -cancel' throws the recording away."

if ($PSCmdlet.ParameterSetName -eq 'Start') {
    Assert-Passed (Invoke-Capped $Wpr $StartArguments) $wprCapLine
    Write-Host "recording; -Stop writes it to $etl"
    exit 0
}

Assert-Passed (Invoke-Capped $Wpr "-stop `"$etl`"") $wprCapLine
if (-not (Test-Path -LiteralPath $etl -PathType Leaf)) {
    Exit-Refused "wpr exited 0 but wrote no file at $etl"
}
Write-Host "trace file: $etl"
Write-Host "trace size: $((Get-Item -LiteralPath $etl).Length) bytes"

$dumpCapLine = "xperf did not finish. The .etl is kept; the xperf command above can be run by hand."
Assert-Passed (Invoke-Capped $Xperf "-i `"$etl`" -o `"$text`" -a dumper") $dumpCapLine
if (-not (Test-Path -LiteralPath $text -PathType Leaf)) {
    Exit-Refused "xperf exited 0 but wrote no file at $text"
}
Write-Host "text file: $text"
Write-Host "text size: $((Get-Item -LiteralPath $text).Length) bytes"
exit 0
