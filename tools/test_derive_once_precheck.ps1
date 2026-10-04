#Requires -Version 7.0
<#
Self-test for tools/derive_once_precheck.ps1.

    pwsh tools/test_derive_once_precheck.ps1                 # everything
    pwsh tools/test_derive_once_precheck.ps1 -DisableCheck 2 # must FAIL
    pwsh tools/test_derive_once_precheck.ps1 -FixtureOnly    # skip the two real ranges

Part 1 builds a tiny git repository in a temp folder. Its main branch holds
one test helper and one ADR that decides a 45-second deadline. A feature
branch adds one renamed copy of the helper, one recomputed offset, one
undecided number (37), the decided 45, and one stray source scan. The script
must report the four planted items, one per check, and must not report the
decided 45. Then the fixture runs again once per check with that check
turned off, and each of those runs must miss its planted item, which proves
every check is needed for this test to pass.

Part 2 runs the script on the two first-review ranges of the 2026-10-04
gate (c6debcd^2...c6debcd and 11b9d44^2...11b9d44) in this repository and
checks it names the findings the reviewers wrote by hand. It is skipped, with
a note, when those commits are not in the repository.

-DisableCheck N passes "-Disable N" to the script in every run. The test then
fails, which is the manual proof that it notices a check being turned off.
The script and this test never touch the repository they run in; the
fixture lives in a temp folder that is deleted at the end.
#>
[CmdletBinding()]
param(
    [ValidateRange(1, 4)][int[]]$DisableCheck = @(),
    [switch]$FixtureOnly
)

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$precheck = Join-Path $PSScriptRoot 'derive_once_precheck.ps1'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

function Invoke-Precheck([string]$Repo, [string]$Range, [int[]]$Disable) {
    $args2 = @('-NoProfile', '-File', $precheck, '-Repo', $Repo, '-Range', $Range)
    if ($Disable.Count) { $args2 += @('-Disable', ($Disable -join ',')) }
    $out = & pwsh @args2 2>&1
    if ($LASTEXITCODE -ne 0) { throw "precheck failed on $Range ($LASTEXITCODE): $($out | Out-String)" }
    @($out | ForEach-Object { "$_" } | Where-Object { $_ -match '^[A-E] ' })
}

# One expectation: a line starting with $Kind and $File whose text matches
# $Like. Returns $null when it holds, or the reason when it does not.
function Test-Expect([string[]]$Lines, [string]$Kind, [string]$File, [string]$Like) {
    $hit = $Lines | Where-Object { $_.StartsWith("$Kind ${File}:") -and $_ -match $Like }
    if ($hit) { return $null }
    "no '$Kind $File' line matching /$Like/"
}

function Assert-Expectations([string]$Label, [string[]]$Lines, [object[]]$Expect, [object[]]$Absent) {
    foreach ($e in $Expect) {
        $why = Test-Expect $Lines $e[0] $e[1] $e[2]
        if ($why) { $script:Failures.Add("${Label}: $why") } else { $script:Passes++ }
    }
    foreach ($a in $Absent) {
        $bad = @($Lines | Where-Object { $_ -match $a })
        if ($bad.Count) { $script:Failures.Add("${Label}: this line matches /$a/ but should not: $($bad[0])") }
        else { $script:Passes++ }
    }
    foreach ($l in $Lines) {
        if ($l -notmatch '^[A-E] \S+:\d+  .+  -- .+$') { $script:Failures.Add("${Label}: not one plain item line: $l") }
    }
}

# ------------------------------------------------------------ part 1: fixture

$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ("precheck_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $fixture | Out-Null
function Write-Fixture([string]$Rel, [string]$Text) {
    $p = Join-Path $fixture $Rel
    New-Item -ItemType Directory -Force -Path (Split-Path $p) | Out-Null
    [System.IO.File]::WriteAllText($p, $Text.Replace("`r`n", "`n"))
}
function Invoke-FixtureGit([string[]]$GitArgs) {
    $out = & git -C $fixture -c user.name=precheck -c user.email=precheck@example.invalid -c core.autocrlf=false @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' '): $($out | Out-String)" }
}

try {
    Invoke-FixtureGit @('init', '-q', '-b', 'main')
    Write-Fixture 'tests/helpers_a.cpp' @'
#include <vector>

namespace {

// Adds the positive values and subtracts the negative ones.
int sum_values(const std::vector<int>& values) {
    int total = 0;
    for (int v : values) {
        if (v > 0) total += v;
        else total -= v;
    }
    return total;
}

}  // namespace
'@
    Write-Fixture 'docs/adr/0001-limits.md' @'
# 1. Test limits

The batch test's deadline is 45 seconds.
'@
    Invoke-FixtureGit @('add', '-A')
    Invoke-FixtureGit @('commit', '-q', '-m', 'main')
    Invoke-FixtureGit @('checkout', '-q', '-b', 'feature')
    # Check 1: the same helper with its names changed.
    Write-Fixture 'tests/helpers_b.cpp' @'
#include <vector>

namespace {

int add_up(const std::vector<int>& xs) {
    int acc = 0;
    for (int x : xs) {
        if (x > 0) acc += x;
        else acc -= x;
    }
    return acc;
}

}  // namespace
'@
    # Check 2: an offset recomputed from two times. Check 3: 37 has no
    # decision; 45 is decided by ADR 0001 next to "deadline".
    Write-Fixture 'tests/test_offsets.cpp' @'
#include "doctest.h"

TEST_CASE("offsets") {
    const auto deadline = std::chrono::seconds(45);
    CHECK(rows.size() > 37);
    const double offset = note.timecode.ms() - end.ms();
    CHECK(offset < 0.0);
}
'@
    # Check 4: a test that reads the source tree itself.
    Write-Fixture 'tests/test_stray_scan.cpp' @'
#include <fstream>

TEST_CASE("model.h mentions the window") {
    std::ifstream in(std::string(HYDRA_SOURCE_DIR) + "/src/core/model.h");
    CHECK(in.good());
}
'@
    Invoke-FixtureGit @('add', '-A')
    Invoke-FixtureGit @('commit', '-q', '-m', 'feature')

    $expect = @(
        @('C', 'tests/helpers_b.cpp', 'add_up.*same body.*sum_values'),
        @('B', 'tests/test_offsets.cpp', 'offset worked out from two times'),
        @('D', 'tests/test_offsets.cpp', '\b37 in:'),
        @('E', 'tests/test_stray_scan.cpp', 'HYDRA_SOURCE_DIR')
    )
    $absent = @('\b45 in:', '^[A-E] tests/helpers_a\.cpp:')

    $lines = Invoke-Precheck $fixture 'main...feature' $DisableCheck
    Assert-Expectations 'fixture' $lines $expect $absent

    # Each check, turned off, must lose its own planted item.
    if (-not $DisableCheck.Count) {
        foreach ($n in 1..4) {
            $off = Invoke-Precheck $fixture 'main...feature' @($n)
            $e = $expect[$n - 1]
            if (Test-Expect $off $e[0] $e[1] $e[2]) { $script:Passes++ }
            else { $script:Failures.Add("fixture with check $n off: still reports its planted item, so the test cannot see check $n") }
        }
    }
} finally {
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

# ------------------------------------------------- part 2: the two real ranges

if (-not $FixtureOnly) {
    & git -C $repoRoot cat-file -e 'c6debcd^{commit}' 2>$null
    $haveS1 = $LASTEXITCODE -eq 0
    & git -C $repoRoot cat-file -e '11b9d44^{commit}' 2>$null
    $haveS2 = $LASTEXITCODE -eq 0
    if ($haveS1) {
        $s1 = Invoke-Precheck $repoRoot 'c6debcd^2...c6debcd' $DisableCheck
        Assert-Expectations 'step 1' $s1 @(
            @('C', 'tests/test_fast_tempo.cpp', 'collect_variants.*test_search\.cpp'),
            @('C', 'tests/test_fast_tempo.cpp', 'windows_text.*test_search\.cpp'),
            @('B', 'tests/test_search.cpp', 'plusmeasure\(act\.timecode'),
            @('B', 'tests/test_search.cpp', 'fabs.*500\.0'),
            @('B', 'tests/test_search.cpp', 'early == 1\.0'),
            @('D', 'src/search/engine.cpp', '0xFFFF'),
            @('D', 'tests/test_fast_tempo.cpp', 'seed <= 48'),
            @('E', 'tests/test_model.cpp', 'HYDRA_SOURCE_DIR'),
            @('E', 'tests/test_preview_view.cpp', 'preview_view\.cpp')
        ) @()
    } else { Write-Host 'step 1 range skipped: c6debcd is not in this repository' }
    if ($haveS2) {
        $s2 = Invoke-Precheck $repoRoot '11b9d44^2...11b9d44' $DisableCheck
        Assert-Expectations 'step 2' $s2 @(
            @('C', 'tests/test_s2_parser_owners.cpp', 'deflate_raw.*test_srb\.cpp'),
            @('C', 'tests/test_s2_offspeed.cpp', 'fill_store.*test_dm_report\.cpp'),
            @('C', 'tests/test_s2_disco.cpp', 'chart_with.*test_song\.cpp'),
            @('C', 'tests/test_s2_dynamics_tag.cpp', 'MThd'),
            @('B', 'tests/test_song.cpp', 'want = read =='),
            @('B', 'tools/ch_probe/tests/test_s2_window_constants.py', 'WINDOW_CAP_MS / 2'),
            @('D', 'src/parse/song.cpp', '\b84 in:'),
            @('E', 'tests/test_s2_stamps.cpp', 'stored_versions\.h')
        ) @()
    } else { Write-Host 'step 2 range skipped: 11b9d44 is not in this repository' }
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_derive_once_precheck: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_derive_once_precheck: PASS, $($script:Passes) checks"
exit 0
