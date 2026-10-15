# Checks that the CTest slices of hydra_tests run every test case exactly once.
#
# The slices are owned by tests/shards.txt, and CMakeLists.txt turns them into
# CTest entries. This script reads neither. It asks ctest for the commands it
# would actually run, takes every entry whose program is hydra_tests, and lists
# the cases each one selects with doctest's -ltc. Those lists together must
# equal the whole exe's list: no case missing, no case in two entries.
#
# It also checks the exe is not older than the test files. It reads the
# hydra_test_sources glob from CMakeLists.txt, which owns the pattern, and
# every file that glob matches must give the whole exe at least one case.
# A file with none usually means the exe was built before the file was
# picked up, so the fix is to rebuild. The one exemption is named below.
#
# CTest runs it as hydra_tests_shards_cover_all. By hand, from the repo root:
#   pwsh -NoProfile -File tools\check_shards.ps1 -Ctest <ctest.exe> `
#        -BuildDir build-cpp -Config Release -Exe build-cpp\Release\hydra_tests.exe
#
# Exit 0 when the slices cover the suite and every test file has cases, 1 when
# either check fails, 2 when a listing could not be made or the glob could not
# be read.

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

$sourceRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# The test files the hydra_test_sources glob in CMakeLists.txt matches, as
# normalized full paths. The glob's line is read, not copied, so the pattern
# keeps one owner.
function Get-GlobbedTestFiles {
    $cmakeLists = Join-Path $sourceRoot 'CMakeLists.txt'
    if (-not (Test-Path -LiteralPath $cmakeLists)) { Fail 2 "no CMakeLists.txt at $cmakeLists" }
    $text = Get-Content -LiteralPath $cmakeLists -Raw
    $m = [regex]::Match($text, '(?m)^\s*file\(\s*GLOB\s+hydra_test_sources\s+([^)]*)\)')
    if (-not $m.Success) { Fail 2 "no file(GLOB hydra_test_sources ...) line in $cmakeLists" }
    $words = @($m.Groups[1].Value -split '\s+' | Where-Object { $_ -and $_ -ne 'CONFIGURE_DEPENDS' })
    if ($words.Count -ne 1) {
        Fail 2 "expected one pattern in the hydra_test_sources glob, found: $($m.Value)"
    }
    $pattern = $words[0].Replace('${CMAKE_SOURCE_DIR}', $sourceRoot).Replace('${CMAKE_CURRENT_SOURCE_DIR}', $sourceRoot)
    if ($pattern.Contains('${')) { Fail 2 "the hydra_test_sources glob uses a variable this script cannot expand: $($m.Value)" }
    if (-not [System.IO.Path]::IsPathRooted($pattern)) { $pattern = Join-Path $sourceRoot $pattern }
    $dir = Split-Path $pattern -Parent
    $leaf = Split-Path $pattern -Leaf
    if ([System.Management.Automation.WildcardPattern]::ContainsWildcardCharacters($dir)) {
        Fail 2 "the hydra_test_sources glob has a wildcard in its folder, which this script does not handle: $($m.Value)"
    }
    $files = @(Get-ChildItem -LiteralPath $dir -File | Where-Object { $_.Name -like $leaf } |
        ForEach-Object { Normalize-Path $_.FullName })
    if ($files.Count -eq 0) { Fail 2 "the hydra_test_sources glob matches no files: $($m.Value)" }
    return , $files
}

# Files the glob matches that hold no test cases on purpose.
$noCaseFiles = @(
    # The doctest runner: it defines main() and holds no cases.
    (Normalize-Path (Join-Path $sourceRoot 'tests\test_main.cpp'))
)

# One key per test case: its file, line and name, as doctest's XML listing
# gives them. When $files is given, each case's file name is counted in it too.
# --no-skip lists the cases marked to be skipped as well: a file whose only
# cases are skipped ones still reached the exe, and is not a stale build.
function Get-CaseKeys([string] $program, [string[]] $arguments, [string] $label, [hashtable] $files = $null) {
    $out = & $program @arguments -ltc --no-skip --reporters=xml 2>&1
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
        if ($null -ne $files) { $files[$tc.filename] = 1 + [int]$files[$tc.filename] }
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

$wholeFiles = @{}
$whole = Get-CaseKeys $exePath @() 'the whole exe' $wholeFiles

# doctest gives file names relative to the source root when the compiler trims
# the prefix (CMakeLists.txt's /d1trimfile), and full paths otherwise.
$filesWithCases = @{}
foreach ($f in $wholeFiles.Keys) {
    $full = if ([System.IO.Path]::IsPathRooted($f)) { $f } else { Join-Path $sourceRoot $f }
    $filesWithCases[(Normalize-Path $full)] = $true
}
$globbed = Get-GlobbedTestFiles
$emptyFiles = @($globbed | Where-Object { -not $filesWithCases.ContainsKey($_) -and $noCaseFiles -notcontains $_ } | Sort-Object)

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

Write-Host "test files the glob matches: $($globbed.Count); with no case in the exe: $($emptyFiles.Count)"
foreach ($f in $emptyFiles) { Write-Host "  no case in the exe: $f" }

if ($emptyFiles.Count -gt 0) {
    Write-Host ('check_shards: the exe has no case from {0} test file(s); it was probably built before they were picked up. Rebuild hydra_tests and run this again.' -f $emptyFiles.Count)
}
if ($missing.Count -gt 0 -or $twice.Count -gt 0) {
    Fail 1 'the slices do not cover the suite exactly once; fix tests/shards.txt'
}
if ($emptyFiles.Count -gt 0) { exit 1 }
Write-Host 'check_shards: every case runs in exactly one entry, and every test file has cases'
exit 0
