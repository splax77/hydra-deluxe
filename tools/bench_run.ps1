# Runs one timing script under the machine-wide benchmark lock, so only one
# timing run happens at a time across every agent's worktree.
#
#   pwsh -NoProfile -File <this file> -Label "<task>-<what>" -Script <path to .ps1>
#
# Exit codes: the timing script's own exit code, or 3 when the lock stayed
# busy for -MaxWaitSeconds (try again later; do other work meanwhile).
# The lock file and bench_log.md live in the hooks state folder
# (~\.claude\hooks\state\bench), beside the build-slot locks, so the repo never
# collects logs. Every run is logged there, with how busy the machine was
# (compiler processes) when it started. The speedups plan's "Proving identical
# results" section says how the timing runs use it.
param(
    [Parameter(Mandatory)] [string]$Label,
    [Parameter(Mandatory)] [string]$Script,
    [int]$MaxWaitSeconds = 150,
    [int]$QuietWaitSeconds = 90
)
$ErrorActionPreference = 'Stop'
$dir = Join-Path $HOME '.claude\hooks\state\bench'
New-Item -ItemType Directory -Force -Path $dir | Out-Null
$lock = Join-Path $dir 'bench.lock'
$log = Join-Path $dir 'bench_log.md'
$start = Get-Date
$fs = $null
while ($true) {
    try {
        $fs = [System.IO.File]::Open($lock, 'CreateNew', 'Write', 'None')
        $bytes = [System.Text.Encoding]::UTF8.GetBytes("$PID`n$Label`n")
        $fs.Write($bytes, 0, $bytes.Length); $fs.Flush()
        break
    } catch [System.IO.IOException] {
        $owner = (Get-Content $lock -ErrorAction SilentlyContinue | Select-Object -First 1) -as [int]
        if ($owner -and -not (Get-Process -Id $owner -ErrorAction SilentlyContinue)) {
            Remove-Item $lock -Force -ErrorAction SilentlyContinue; continue
        }
        if (((Get-Date) - $start).TotalSeconds -ge $MaxWaitSeconds) {
            $who = (Get-Content $lock -ErrorAction SilentlyContinue) -join ' '
            Write-Output "BENCH LOCK BUSY after $MaxWaitSeconds s (held by: $who). Exit 3: do other work, then retry."
            exit 3
        }
        Start-Sleep -Seconds 5
    }
}
try {
    # Wait (bounded) for compilers to finish so timings are not skewed.
    $busyNames = 'cl', 'link', 'MSBuild', 'clang-cl', 'lld-link', 'hydra_batch', 'hydra_bench', 'hydra_tests'
    $q0 = Get-Date
    while ((Get-Process -Name $busyNames -ErrorAction SilentlyContinue) -and ((Get-Date) - $q0).TotalSeconds -lt $QuietWaitSeconds) {
        Start-Sleep -Seconds 5
    }
    $busy = @(Get-Process -Name $busyNames -ErrorAction SilentlyContinue).Count
    $t = Get-Date
    Write-Output "== bench '$Label' start $($t.ToString('HH:mm:ss')), compiler processes running: $busy"
    $out = & pwsh -NoProfile -File $Script 2>&1 | Out-String
    $code = $LASTEXITCODE
    Write-Output $out
    $secs = [math]::Round(((Get-Date) - $t).TotalSeconds, 1)
    Add-Content -Path $log -Value "## $($t.ToString('HH:mm:ss')) $Label (exit $code, $secs s, compilers busy at start: $busy)`n``````n$($out.Trim())`n```````n"
    exit $code
} finally {
    if ($fs) { $fs.Dispose() }
    Remove-Item $lock -Force -ErrorAction SilentlyContinue
}
