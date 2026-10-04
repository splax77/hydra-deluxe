#Requires -Version 7.0
<#
Self-test for tools/derive_once_precheck.ps1.

    pwsh tools/test_derive_once_precheck.ps1                 # everything
    pwsh tools/test_derive_once_precheck.ps1 -DisableCheck 2 # must FAIL
    pwsh tools/test_derive_once_precheck.ps1 -FixtureOnly    # skip the two real ranges

Part 1 builds a tiny git repository in a temp folder. Its main branch holds
one test helper, one ADR that decides a 45-second deadline, and a copy of
this repository's tests/test_single_owner.cpp, so the scan rows the script
reads are today's. A feature branch adds one renamed copy of the helper, one
recomputed offset, one undecided number (37), the decided 45, one stray
source scan, a new scan row with no must-match examples, and two lines the
scan rows list as must-not-match (a gap between two chart ticks, a "!= SqIn"
skip). The script must report the five planted items (check 4 has two), and
must not report the decided 45 or the two
must-not-match lines. Then the fixture runs again once per check with that check
turned off, and each of those runs must miss its planted item, which proves
every check is needed for this test to pass.

Part 2 runs the script on the two first-review ranges of the 2026-10-04
gate (c6debcd^2...c6debcd and 11b9d44^2...11b9d44) in this repository and
checks it names the findings the reviewers wrote by hand. It reads the scan
rows at this repository's HEAD (-RulesAt HEAD), because the rows that name
those findings were added after those commits. It is skipped, with
a note, when those commits are not in the repository. Then it runs the
script on the whole tree at HEAD, where the scan passes, and fails on any
row hit that is not a line the scan lists (an owner line or a known copy):
that is the script reading a row differently from the scan. Every run fails
on a "precheck:" warning line instead of dropping it.

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

function Invoke-Precheck([string]$Repo, [string]$Range, [int[]]$Disable, [string]$RulesAt = '', [switch]$WholeTree) {
    $args2 = @('-NoProfile', '-File', $precheck, '-Repo', $Repo, '-Range', $Range)
    if ($Disable.Count) { $args2 += @('-Disable', ($Disable -join ',')) }
    if ($RulesAt) { $args2 += @('-RulesAt', $RulesAt) }
    if ($WholeTree) { $args2 += '-WholeTree' }
    $out = @(& pwsh @args2 2>&1 | ForEach-Object { "$_" })
    if ($LASTEXITCODE -ne 0) { throw "precheck failed on $Range ($LASTEXITCODE): $($out | Out-String)" }
    # A "precheck:" line says the script read the scan rows differently from
    # the scan, so it fails the test instead of being dropped (M0 review 2,
    # finding 1).
    foreach ($w in ($out | Where-Object { $_ -match '^precheck: ' })) { $script:Failures.Add("${Range}: the precheck warned: $w") }
    @($out | Where-Object { $_ -match '^[A-E] ' })
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

# ------------------------------------------- part 0: the scan structs' order
#
# The precheck reads each scan row by position, in the member order its
# $ScanStructFields table gives. That table is read here from the precheck's
# own text (as a literal, without running the script), and each struct's
# member names are read, in order, from tests/test_single_owner.cpp. A member
# added, removed or moved there fails here by name, instead of as a pile of
# "precheck:" warnings.
$tokens = $null; $parseErrors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile($precheck, [ref]$tokens, [ref]$parseErrors)
# One definition in the precheck's text, by name: a variable's assignment
# ('$cppLex') or a function ('Blank'). The self-test reads the precheck's own
# owners this way instead of keeping copies of them.
function Find-PrecheckAst([string]$Name) {
    $found = $ast.Find({ param($n)
        ($n -is [System.Management.Automation.Language.AssignmentStatementAst] -and "$($n.Left)" -eq $Name) -or
        ($n -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $n.Name -eq $Name) }, $true)
    if (-not $found) { throw "the self-test cannot find $Name in $precheck" }
    $found
}
$structFields = (Find-PrecheckAst '$ScanStructFields').Right.Expression.SafeGetValue()
# The precheck's C++ lexer and its comment blanking, loaded from its text, so
# a comment is whatever the precheck says it is (strings included).
. ([scriptblock]::Create((@('$cppLex', 'Blank', 'Remove-Comments') | ForEach-Object { (Find-PrecheckAst $_).Extent.Text }) -join "`n"))
# The scan file's path is the precheck's $scanFile; this repository's copy is
# read once, for the struct check here and for the fixture below.
$scanFile = (Find-PrecheckAst '$scanFile').Right.Expression.SafeGetValue()
$repoScanText = [System.IO.File]::ReadAllText((Join-Path $repoRoot $scanFile))
$scanSource = Remove-Comments $cppLex $repoScanText
foreach ($struct in $structFields.Keys) {
    $m = [regex]::Match($scanSource, "\bstruct\s+$struct\s*\{(?<body>[^{}]*)\}\s*;")
    $inStruct = @(if ($m.Success) {
        foreach ($decl in ($m.Groups['body'].Value -split ';')) {
            $d = ($decl -replace '=[\s\S]*$', '').Trim()
            if ($d -match '([A-Za-z_]\w*)$') { $Matches[1] }
        }
    })
    $inPrecheck = @($structFields[$struct])
    if (($inStruct -join ',') -ne ($inPrecheck -join ',')) {
        $script:Failures.Add("${struct}'s fields changed; update the column positions in derive_once_precheck.ps1 " +
            "(tests/test_single_owner.cpp has: $(if ($inStruct.Count) { $inStruct -join ', ' } else { 'no such struct' }); " +
            "`$ScanStructFields has: $($inPrecheck -join ', '))")
    } else { $script:Passes++ }
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
    # The scan rows come from this repository's own scan file, so the
    # fixture tests the rows as they are today and copies none of them.
    Write-Fixture $scanFile $repoScanText
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
    # Check 2: an offset recomputed from two times, which the scan row "How
    # far is a note from an SP end, in a test?" flags. The gap between two
    # chart ticks and the "!= SqIn" skip are must-not-match lines of their
    # rows, so the precheck must pass them as the scan does (M0 review 1,
    # finding 1). Check 3: 37 has no decision; 45 is decided by ADR 0001 next
    # to "deadline".
    Write-Fixture 'tests/test_offsets.cpp' @'
#include "doctest.h"

TEST_CASE("offsets") {
    const auto deadline = std::chrono::seconds(45);
    CHECK(rows.size() > 37);
    const double offset = note.timecode.ms() - end.ms();
    CHECK(offset < 0.0);
    double gap = st.timecode(69120).ms() - st.timecode(68880).ms();
    for (const auto& s : steps) {
        if (s.kind != SpEndKind::SqIn) continue;
    }
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
    # Check 4 again: a new scan row with no must-match examples, read from
    # the rows the script already parsed (M0 review 2, finding 3).
    $tableOpen = 'static const std::vector<OwnerRule> r = {'
    if (-not $repoScanText.Contains($tableOpen)) { throw "the fixture cannot find '$tableOpen' in $scanFile" }
    Write-Fixture $scanFile ($repoScanText.Replace($tableOpen, $tableOpen +
        "`n        {""Which planted row has no examples?"", ""nobody"", R""re(\bplanted_no_examples\b)re"", """", {}, {}, ""fixture"", {}, {""int planted_must_not = 0;""}},"))
    Invoke-FixtureGit @('add', '-A')
    Invoke-FixtureGit @('commit', '-q', '-m', 'feature')

    $expect = @(
        @('C', 'tests/helpers_b.cpp', 'add_up.*same body.*sum_values'),
        @('B', 'tests/test_offsets.cpp', 'How far is a note from an SP end.*end\.ms\(\)'),
        @('D', 'tests/test_offsets.cpp', '\b37 in:'),
        @('E', 'tests/test_stray_scan.cpp', 'HYDRA_SOURCE_DIR'),
        @('E', 'tests/test_single_owner.cpp', 'planted row has no examples.*no must-match examples')
    )
    $absent = @('\b45 in:', '^[A-E] tests/helpers_a\.cpp:', '^[A-E] tests/test_offsets\.cpp:\d+  .*timecode\(69120\)',
                '^[A-E] tests/test_offsets\.cpp:\d+  .*SpEndKind::SqIn')

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

# Is this commit in this repository? A shallow or partial clone may lack the
# two old ranges.
function Test-HaveCommit([string]$Name) {
    & git -C $repoRoot cat-file -e "$Name^{commit}" 2>$null
    $LASTEXITCODE -eq 0
}

if (-not $FixtureOnly) {
    $haveS1 = Test-HaveCommit 'c6debcd'
    $haveS2 = Test-HaveCommit '11b9d44'
    if ($haveS1) {
        $s1 = Invoke-Precheck $repoRoot 'c6debcd^2...c6debcd' $DisableCheck 'HEAD'
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
        $s2 = Invoke-Precheck $repoRoot '11b9d44^2...11b9d44' $DisableCheck 'HEAD'
        Assert-Expectations 'step 2' $s2 @(
            @('C', 'tests/test_s2_parser_owners.cpp', 'deflate_raw.*test_srb\.cpp'),
            @('C', 'tests/test_s2_offspeed.cpp', 'fill_store.*test_dm_report\.cpp'),
            @('C', 'tests/test_s2_disco.cpp', 'chart_with.*test_song\.cpp'),
            @('C', 'tests/test_s2_dynamics_tag.cpp', 'MThd'),
            @('B', 'tests/test_song.cpp', 'want = read =='),
            @('B', 'tools/ch_probe/tests/test_s2_window_constants.py', 'WINDOW_CAP_MS / 2'),
            @('D', 'tools/ch_probe/tests/test_s2_window_constants.py', '\b75 in:'),
            @('E', 'tests/test_s2_stamps.cpp', 'stored_versions\.h')
        ) @()
    } else { Write-Host 'step 2 range skipped: 11b9d44 is not in this repository' }

    # The whole tree at HEAD. The scan passes at every commit on main, so it
    # is the truth here: every row hit the precheck prints must be a line the
    # scan lists (an owner line or a known copy). A row hit with neither note
    # is a line the scan passes and the precheck flags, which means the
    # precheck read a row differently (M0 review 2, finding 1).
    $wt = Invoke-Precheck $repoRoot 'HEAD' $DisableCheck -WholeTree
    $bare = @($wt | Where-Object { $_ -match 'the scan row in tests/test_single_owner\.cpp gives this question to' -and
                                   $_ -notmatch '\((known copy: |this range lists it as an owner line: )' })
    if ($bare.Count) { foreach ($b in $bare) { $script:Failures.Add("whole tree at HEAD: the scan passes this line, the precheck flags it: $b") } }
    else { $script:Passes++ }
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_derive_once_precheck: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_derive_once_precheck: PASS, $($script:Passes) checks"
exit 0
