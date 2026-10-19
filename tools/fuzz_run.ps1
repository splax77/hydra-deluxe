# Runs the libFuzzer exes the fuzz preset builds, in one of two ways, and
# holds the per-input limits both ways share, so a hang or a memory blowup
# means the same thing in each.
#
#   fuzz    grows one target's corpus for a fixed time (fuzz.yml's fuzz job).
#   replay  runs every saved input once, with no mutation, and stops (the
#           fuzz-replay job in ci.yml).
#
# Replay takes one or more input roots. Each root holds one folder per target,
# named as the target is: testdata/fuzz (its README.md says so) and the
# combined grown corpus fuzz.yml saves. Replay runs exactly the targets those
# folder names give, so it never keeps a list of its own; the list is kTargets
# in tests/fuzz/harness.h. A folder with no exe to run it fails the run.
#
# libFuzzer's exit code is nonzero on a crash, a hang past the timeout or a
# memory blowup past the limit, and this script exits nonzero when any run did.
#
# Usage:
#   tools\fuzz_run.ps1 -Mode fuzz -Target chart -Corpus corpus -Seeds seeds\chart -Seconds 600 -CrashDir crashes
#   tools\fuzz_run.ps1 -Mode replay -InputRoot corpus-all, testdata\fuzz -CrashDir crashes
# The exes load the AddressSanitizer runtime, so run this from a Visual Studio
# developer shell. See "Fuzzing the file parsers" in docs/development.md.

param(
    [Parameter(Mandatory)][ValidateSet('fuzz', 'replay')][string]$Mode,
    [string]$ExeDir = '',
    [string]$CrashDir = 'crashes',
    # fuzz mode
    [string]$Target = '',
    [string]$Corpus = '',
    [string]$Seeds = '',
    [int]$Seconds = 0,
    # replay mode
    [string[]]$InputRoot = @()
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $ExeDir) { $ExeDir = Join-Path $repo 'build-fuzz\Release' }

# The per-input limits, in seconds and megabytes.
$limits = @('-timeout=10', '-rss_limit_mb=2048')

New-Item -ItemType Directory -Force $CrashDir | Out-Null
$artifactPrefix = '-artifact_prefix=' + (Join-Path (Resolve-Path $CrashDir).Path '')

function Get-FuzzExe([string]$name) {
    $exe = Join-Path $ExeDir "fuzz_$name.exe"
    if (-not (Test-Path -LiteralPath $exe)) { return $null }
    return $exe
}

if ($Mode -eq 'fuzz') {
    if (-not $Target -or -not $Corpus -or -not $Seeds -or $Seconds -le 0) {
        Write-Error 'fuzz_run: fuzz mode needs -Target, -Corpus, -Seeds and -Seconds.'
    }
    $exe = Get-FuzzExe $Target
    if (-not $exe) { Write-Error "fuzz_run: no fuzz_$Target.exe in $ExeDir" }
    New-Item -ItemType Directory -Force $Corpus | Out-Null
    $extra = @()
    $dict = Join-Path $repo "tests\fuzz\$Target.dict"
    if (Test-Path -LiteralPath $dict) { $extra += "-dict=$dict" }
    # libFuzzer adds new inputs to the first folder and only reads the rest.
    & $exe $Corpus $Seeds "-max_total_time=$Seconds" @limits $artifactPrefix @extra
    exit $LASTEXITCODE
}

# replay
if ($InputRoot.Count -eq 0) { Write-Error 'fuzz_run: replay mode needs -InputRoot.' }
$roots = @($InputRoot | Where-Object { Test-Path -LiteralPath $_ -PathType Container })
foreach ($r in $InputRoot) {
    if ($roots -notcontains $r) { Write-Host "fuzz_run: input root $r does not exist; skipped." }
}
$names = @($roots | ForEach-Object { Get-ChildItem -LiteralPath $_ -Directory } |
    ForEach-Object Name | Sort-Object -Unique)
if ($names.Count -eq 0) { Write-Error 'fuzz_run: no target folders under any input root.' }

$failed = @()
foreach ($name in $names) {
    $exe = Get-FuzzExe $name
    if (-not $exe) {
        Write-Host "fuzz_run: FAIL $name - a folder names it but $ExeDir has no fuzz_$name.exe"
        $failed += $name
        continue
    }
    $dirs = @($roots | ForEach-Object { Join-Path $_ $name } |
        Where-Object { Test-Path -LiteralPath $_ -PathType Container })
    # libFuzzer skips empty files, such as the .gitkeep placeholders.
    $count = @($dirs | ForEach-Object { Get-ChildItem -LiteralPath $_ -File } |
        Where-Object Length -gt 0).Count
    Write-Host "fuzz_run: replaying $count inputs for $name from $($dirs -join ', ')"
    # -runs=0: run every input in the folders once, then exit without mutating.
    & $exe @dirs '-runs=0' @limits $artifactPrefix
    if ($LASTEXITCODE -ne 0) {
        Write-Host "fuzz_run: FAIL $name - exit code $LASTEXITCODE"
        $failed += $name
    }
}
if ($failed.Count -gt 0) {
    Write-Host "fuzz_run: failed targets: $($failed -join ', ')"
    exit 1
}
Write-Host "fuzz_run: every saved input ran clean for: $($names -join ', ')"
exit 0
