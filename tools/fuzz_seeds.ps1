# Writes the seed folders the fuzz targets start from: one folder per target
# (tests/fuzz/harness.h, kTargets), filled from files already checked in.
#
# The .chart and .mid seeds are the notes files under testdata/input, audio is
# testdata/audio/sine220.*, and the image seed is testdata/preview/golden_onyx.png.
# No .sng or .srb is checked in, so hydra_tests writes those: the case
# "fuzz seeds" in tests/test_fuzz_regressions.cpp wraps corpus charts with the
# tests' own fixture builders (tests/sng_util.h, tests/srb_util.h). That needs
# a built hydra_tests; any preset's works.
#
# Usage: tools\fuzz_seeds.ps1 [-OutDir build-fuzz\seeds] [-TestsExe <hydra_tests.exe>]
# Then: build-fuzz\Release\fuzz_chart.exe <corpus dir> <OutDir>\chart ...
# See "Fuzzing the file parsers" in docs/development.md.

param(
    [string]$OutDir = "",
    [string]$TestsExe = ""
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $OutDir) { $OutDir = Join-Path $repo 'build-fuzz\seeds' }
if (-not $TestsExe) {
    $TestsExe = @('build-cpp', 'build-asan') |
        ForEach-Object { Join-Path $repo "$_\Release\hydra_tests.exe" } |
        Where-Object { Test-Path $_ } | Select-Object -First 1
}
if (-not $TestsExe -or -not (Test-Path $TestsExe)) {
    Write-Error ("fuzz_seeds: no hydra_tests.exe to write the .sng and .srb seeds. " +
                 "Build one (build_cpp.ps1 -Target hydra_tests) or pass -TestsExe.")
}

if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force $OutDir | Out-Null

# Copies each file into <OutDir>\<target> under a numbered name; the folders
# the files came from can share names, the seeds cannot.
function Copy-Seeds([string]$Target, [System.IO.FileInfo[]]$Files) {
    $dir = Join-Path $OutDir $Target
    New-Item -ItemType Directory -Force $dir | Out-Null
    $i = 0
    foreach ($f in $Files) {
        Copy-Item -LiteralPath $f.FullName (Join-Path $dir ('{0:d4}{1}' -f $i, $f.Extension))
        $i++
    }
    if ($i -eq 0) { Write-Error "fuzz_seeds: no seeds found for $Target" }
}

$inputDir = Join-Path $repo 'testdata\input'
Copy-Seeds 'chart' @(Get-ChildItem -LiteralPath $inputDir -Recurse -File -Filter 'notes.chart')
Copy-Seeds 'mid' @(Get-ChildItem -LiteralPath $inputDir -Recurse -File -Filter 'notes.mid')
Copy-Seeds 'audio' @(Get-ChildItem -Path (Join-Path $repo 'testdata\audio\sine220.*') -File)
Copy-Seeds 'image' @(Get-Item -LiteralPath (Join-Path $repo 'testdata\preview\golden_onyx.png'))

$env:HYDRA_FUZZ_SEED_DIR = (Resolve-Path $OutDir).Path
try {
    & $TestsExe '-tc=fuzz seeds*' | Out-Host
    if ($LASTEXITCODE -ne 0) { Write-Error "fuzz_seeds: hydra_tests failed writing the .sng and .srb seeds" }
} finally {
    Remove-Item Env:HYDRA_FUZZ_SEED_DIR
}
foreach ($t in 'sng', 'srb') {
    if (@(Get-ChildItem -LiteralPath (Join-Path $OutDir $t) -File -ErrorAction SilentlyContinue).Count -eq 0) {
        Write-Error "fuzz_seeds: hydra_tests wrote no $t seeds"
    }
}

Get-ChildItem -LiteralPath $OutDir -Directory | ForEach-Object {
    "{0,-6} {1,3} seeds" -f $_.Name, @(Get-ChildItem -LiteralPath $_.FullName -File).Count
}
