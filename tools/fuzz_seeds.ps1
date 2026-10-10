# Writes the seed folders the fuzz targets start from: one folder per target
# (tests/fuzz/harness.h, kTargets), filled from files already checked in.
#
# hydra_tests does the writing, because the only builders of the .sng and .srb
# containers are the test fixtures: the case "fuzz seeds" in
# tests/test_fuzz_regressions.cpp says which files seed which target, and fails
# when a target gets none. This script only runs it. That needs a built
# hydra_tests; any preset's works.
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
    Write-Error ("fuzz_seeds: no hydra_tests.exe to write the seeds with. " +
                 "Build one (build_cpp.ps1 -Target hydra_tests) or pass -TestsExe.")
}

if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force $OutDir | Out-Null

$env:HYDRA_FUZZ_SEED_DIR = (Resolve-Path $OutDir).Path
try {
    & $TestsExe '-tc=fuzz seeds*' | Out-Host
    if ($LASTEXITCODE -ne 0) { Write-Error "fuzz_seeds: hydra_tests failed writing the seeds" }
} finally {
    Remove-Item Env:HYDRA_FUZZ_SEED_DIR
}

Get-ChildItem -LiteralPath $OutDir -Directory | ForEach-Object {
    "{0,-6} {1,3} seeds" -f $_.Name, @(Get-ChildItem -LiteralPath $_.FullName -File).Count
}
