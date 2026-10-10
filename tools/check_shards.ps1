# Checks that the CTest slices of hydra_tests run every test case exactly once.
#
# The slices are owned by tests/shards.txt, and CMakeLists.txt turns them into
# CTest entries. This script reads neither. It asks ctest for the commands it
# would actually run, takes every entry whose program is hydra_tests, and lists
# the cases each one selects with doctest's -ltc. Those lists together must
# equal the whole exe's list: no case missing, no case in two entries.
#
# CTest runs it as hydra_tests_shards_cover_all. By hand, from the repo root:
#   pwsh -NoProfile -File tools\check_shards.ps1 -Ctest <ctest.exe> `
#        -BuildDir build-cpp -Config Release -Exe build-cpp\Release\hydra_tests.exe
#
# Exit 0 when the slices cover the suite, 1 when they do not, 2 when a listing
# could not be made.

param(
    [Parameter(Mandatory)] [string] $Ctest,
    [Parameter(Mandatory)] [string] $BuildDir,
    [Parameter(Mandatory)] [string] $Config,
    [Parameter(Mandatory)] [string] $Exe
)

$ErrorActionPreference = 'Stop'
# doctest prints test names as UTF-8.
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

function Fail([int] $code, [string] $message) {
    Write-Host "check_shards: $message"
    exit $code
}

function Normalize-Path([string] $path) {
    return [System.IO.Path]::GetFullPath($path).Replace('/', '\').ToLowerInvariant()
}

# One key per test case: its file, line and name, as doctest's XML listing
# gives them.
function Get-CaseKeys([string] $program, [string[]] $arguments, [string] $label) {
    $out = & $program @arguments -ltc --reporters=xml 2>&1
    if ($LASTEXITCODE -ne 0) {
        Fail 2 "listing the cases of $label failed (exit $LASTEXITCODE): $($out -join "`n")"
    }
    try {
        $xml = [xml]($out -join "`n")
    } catch {
        Fail 2 "the case listing of $label is not XML: $($_.Exception.Message)"
    }
    $keys = @()
    foreach ($tc in $xml.doctest.TestCase) {
        $keys += "$($tc.filename):$($tc.line): $($tc.name)"
    }
    return , $keys
}

$exePath = Normalize-Path $Exe
if (-not (Test-Path -LiteralPath $exePath)) {
    Fail 2 "no hydra_tests exe at $Exe"
}

$json = & $Ctest --test-dir $BuildDir -C $Config --show-only=json-v1 2>&1
if ($LASTEXITCODE -ne 0) {
    Fail 2 "ctest --show-only failed (exit $LASTEXITCODE): $($json -join "`n")"
}
$tests = ($json -join "`n" | ConvertFrom-Json).tests

$entries = @()
foreach ($t in $tests) {
    if (-not $t.command -or $t.command.Count -lt 1) { continue }
    if ((Normalize-Path $t.command[0]) -ne $exePath) { continue }
    $entries += [pscustomobject]@{
        Name = $t.name
        Args = @($t.command | Select-Object -Skip 1)
    }
}
if ($entries.Count -eq 0) {
    Fail 2 "ctest has no entry that runs $Exe"
}

$whole = Get-CaseKeys $exePath @() 'the whole exe'
$wholeCount = @{}
foreach ($k in $whole) { $wholeCount[$k] = 1 + [int]$wholeCount[$k] }

$seen = @{}
$where = @{}
foreach ($e in $entries) {
    $keys = Get-CaseKeys $exePath $e.Args $e.Name
    Write-Host ("{0,-32} {1,5} cases  ({2})" -f $e.Name, $keys.Count, ($e.Args -join ' '))
    foreach ($k in $keys) {
        $seen[$k] = 1 + [int]$seen[$k]
        $where[$k] = @($where[$k]) + $e.Name | Where-Object { $_ }
    }
}

$missing = @($wholeCount.Keys | Where-Object { [int]$seen[$_] -lt $wholeCount[$_] } | Sort-Object)
$twice = @($seen.Keys | Where-Object { $seen[$_] -gt [int]$wholeCount[$_] } | Sort-Object)

$sum = 0
foreach ($v in $seen.Values) { $sum += $v }
Write-Host "whole exe: $($whole.Count) cases; the $($entries.Count) entries list $sum; missing $($missing.Count); in more than one entry $($twice.Count)"

foreach ($k in $missing) { Write-Host "  missing from every entry: $k" }
foreach ($k in $twice) { Write-Host "  run more than once ($($where[$k] -join ', ')): $k" }

if ($missing.Count -gt 0 -or $twice.Count -gt 0) {
    Fail 1 'the slices do not cover the suite exactly once; fix tests/shards.txt'
}
Write-Host 'check_shards: every case runs in exactly one entry'
exit 0
