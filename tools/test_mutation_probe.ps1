#Requires -Version 7.0
<#
Self-test for tools/mutation_probe.ps1.

    pwsh -NoProfile -File tools/test_mutation_probe.ps1

Part 1 loads the probe's finder from its own text (as a literal, without
running the script) and runs it on a sample snippet. The snippet has one of
each edit the probe makes and a trap for each thing it must skip: comments,
strings, a raw string, a character literal, a preprocessor line, template
brackets, a shift, an exponent, a digit separator and a "+ 10". The expected
candidates are written out by hand. The snippet is checked with Unix and
Windows line endings.

Part 2 runs the whole probe on a tiny git repository in a temp folder, with
a fake build_cpp.ps1 and a fake test program, so it builds no C++ and takes
no real build slot (-SlotDir points into the fixture). The fakes decide each
edit's fate: one edit does not build, three make the fake tests fail, two
pass. The runs check that the probe refuses the main checkout and a source
file that differs from git; that a normal run reports each edit's result
and the score, and leaves the file byte-identical and git-clean; and that a
run which breaks part way (the fake build deletes the test program) still
restores the file and gives its build slot back. The fixture is deleted at
the end; this repository is never touched.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$probe = Join-Path $PSScriptRoot 'mutation_probe.ps1'

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

# ------------------------------------------------------ part 1: the finder

$ast = [System.Management.Automation.Language.Parser]::ParseFile($probe, [ref]$null, [ref]$null)
$names = 'Get-CodeMask', 'Find-MutationCandidates', 'Get-MutatedText', 'Get-MutatedLine', 'Select-Spread'
$defs = foreach ($name in $names) {
    $f = $ast.Find({ param($n) $n -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $n.Name -eq $name }, $true)
    if (-not $f) { throw "the self-test cannot find $name in $probe" }
    $f.Extent.Text
}
. ([scriptblock]::Create($defs -join "`n"))

$snippet = @'
#include <vector>
#if A > B
#endif
// a < b in a comment
/* x == y
   in a block */
std::vector<int> make(int n) {
    std::vector<int> v;
    for (int i = 0; i < n; ++i) v.push_back(i + 1);
    const char* s = "a == b";
    const char c = '>';
    auto r = R"x(p && q)x";
    double e = 1e+1 + 2.5;
    int big = 1'000 + 10;
    int sh = n << 2;
    return v;
}
bool ok(int a, int b) {
    if (a >= b && b != 0) return true;
    return false;
}
int up(int x) { return x+1; }
'@.Replace("`r`n", "`n")

$expected = @(
    '9: `<` to `<=`',
    '9: `+ 1` to nothing',
    '9: `+ 1` to `- 1`',
    '19: `>=` to `>`',
    '19: `&&` to `||`',
    '19: `!=` to `==`',
    '19: `return true` to `return false`',
    '20: `return false` to `return true`',
    '22: `+1` to nothing',
    '22: `+1` to `-1`'
)
foreach ($eol in "`n", "`r`n") {
    $text = $snippet.Replace("`n", $eol)
    $label = if ($eol -eq "`n") { 'LF' } else { 'CRLF' }
    $found = @(Find-MutationCandidates $text)
    $said = @($found | ForEach-Object { "$($_.Line): $($_.Was) to $($_.Now)" })
    Check (($said -join "`n") -eq ($expected -join "`n")) "finder ($label): expected`n  $($expected -join "`n  ")`ngot`n  $($said -join "`n  ")"
    Check ((@($found | ForEach-Object Number) -join ',') -eq '1,2,3,4,5,6,7,8,9,10') "finder ($label): candidates are not numbered 1 to 10 in order"
    if ($found.Count -ge 3) {
        Check ((Get-MutatedLine $text $found[1]) -eq 'for (int i = 0; i < n; ++i) v.push_back(i);') "dropping + 1 ($label) gave: $(Get-MutatedLine $text $found[1])"
        Check ((Get-MutatedLine $text $found[2]) -eq 'for (int i = 0; i < n; ++i) v.push_back(i - 1);') "+ 1 to - 1 ($label) gave: $(Get-MutatedLine $text $found[2])"
        # One edit changes only its own characters.
        $m = Get-MutatedText $text $found[0]
        Check ($m.Length -eq $text.Length + 1 -and $m.Replace('i <= n', 'i < n') -eq $text) "the < to <= edit ($label) changed more than the operator"
    }
}
Check ((@(Select-Spread 6 3) -join ',') -eq '1,3,5') "Select-Spread 6 3 gave $(@(Select-Spread 6 3) -join ',')"
Check ((@(Select-Spread 4 10) -join ',') -eq '1,2,3,4') "Select-Spread 4 10 gave $(@(Select-Spread 4 10) -join ',')"
Check (@(Select-Spread 0 5).Count -eq 0) 'Select-Spread 0 5 is not empty'

# ------------------------------------------------- part 2: runs on a fixture

$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ("mutation_probe_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
$repo = Join-Path $fixture 'repo'
$wt = Join-Path $fixture 'wt'
$slots = Join-Path $fixture 'slots'
$runner = Join-Path $fixture 'fake_tests.ps1'
New-Item -ItemType Directory -Path $repo, $slots | Out-Null

function Invoke-FixtureGit([string]$Dir, [string[]]$GitArgs) {
    $out = & git -C $Dir -c user.name=probe -c user.email=probe@example.invalid -c core.autocrlf=false @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' '): $($out | Out-String)" }
    $out
}
function Write-Text([string]$Path, [string]$Text) {
    New-Item -ItemType Directory -Force -Path (Split-Path $Path) | Out-Null
    [System.IO.File]::WriteAllText($Path, $Text.Replace("`r`n", "`n"))
}
# One probe run in its own pwsh: its exit code and its output lines.
function Invoke-Probe([string]$In, [string]$Exe, [string[]]$Pick = @('-Max', '6')) {
    $out = @(& pwsh -NoProfile -File $probe -Repo $In -Source src/sample.cpp -Filter 'sf=*sample*' @Pick -TestExe $Exe -SlotDir $slots 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $LASTEXITCODE; Lines = $out; Text = ($out -join "`n") }
}

try {
    $sample = @'
int next_index(int i) { return i + 1; }
int clamp_count(int n, int cap) {
    if (n < cap) return n;
    return cap;
}
bool both(bool a, bool b) {
    if (a && b) return true;
    return false;
}
'@
    Write-Text (Join-Path $repo 'src/sample.cpp') $sample
    Write-Text (Join-Path $repo '.gitignore') "build-cpp/`n"
    # The fake build: "i - 1" does not compile; anything else is "built" by
    # copying the source where the fake tests read it. With
    # MUTPROBE_FAKE_CRASH set to a file, its second build deletes that file.
    Write-Text (Join-Path $repo 'build_cpp.ps1') @'
param([string]$Target)
$out = Join-Path $PSScriptRoot 'build-cpp'
New-Item -ItemType Directory -Force -Path $out | Out-Null
if ($env:MUTPROBE_FAKE_CRASH) {
    $count = Join-Path $out 'builds.txt'
    $n = if (Test-Path $count) { [int](Get-Content $count) + 1 } else { 1 }
    Set-Content $count $n
    if ($n -ge 2) { Remove-Item -LiteralPath $env:MUTPROBE_FAKE_CRASH }
}
$text = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'src/sample.cpp'))
if ($text.Contains('i - 1')) { Write-Host 'error C2000: fake compile error'; exit 1 }
Copy-Item (Join-Path $PSScriptRoot 'src/sample.cpp') (Join-Path $out 'built.cpp')
'@
    Invoke-FixtureGit $repo @('init', '-q', '-b', 'main') | Out-Null
    Invoke-FixtureGit $repo @('add', '-A') | Out-Null
    Invoke-FixtureGit $repo @('commit', '-q', '-m', 'fixture') | Out-Null
    Invoke-FixtureGit $repo @('worktree', 'add', '-q', $wt, '-b', 'probe') | Out-Null
    # The fake tests fail on three of the edits and pass on the rest.
    Write-Text $runner (@'
param([string]$Filter)
$built = [IO.File]::ReadAllText('BUILT')
$fail = $built.Contains('n <= cap') -or $built.Contains('a || b') -or $built.Contains('return i;')
if ($fail) { '[doctest] test cases: 3 | 2 passed | 1 failed'; exit 1 }
'[doctest] test cases: 3 | 3 passed | 0 failed'
'@.Replace('BUILT', (Join-Path $wt 'build-cpp\built.cpp')))
    $sampleWt = Join-Path $wt 'src\sample.cpp'
    $committed = [IO.File]::ReadAllBytes($sampleWt)
    $restored = {
        param([string]$Label)
        $same = [Convert]::ToBase64String([IO.File]::ReadAllBytes($sampleWt)) -eq [Convert]::ToBase64String($committed)
        Check $same "${Label}: src/sample.cpp is not byte-identical to the commit"
        $dirty = Invoke-FixtureGit $wt @('status', '--porcelain')
        Check (-not $dirty) "${Label}: git status is not clean: $dirty"
        Check (@(Get-ChildItem $slots).Count -eq 0) "${Label}: a build slot lock was left in $slots"
    }

    # Refusals: the main checkout, and a source file that differs from git.
    $r = Invoke-Probe $repo $runner
    Check ($r.Code -eq 1 -and $r.Text -match 'is the main checkout') "main checkout: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    [IO.File]::AppendAllText($sampleWt, "`n")
    $r = Invoke-Probe $wt $runner
    Check ($r.Code -eq 1 -and $r.Text -match 'differs from git') "changed source: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    Invoke-FixtureGit $wt @('checkout', '--', 'src/sample.cpp') | Out-Null

    # A normal run.
    $r = Invoke-Probe $wt $runner
    Check ($r.Code -eq 0) "normal run: exit $($r.Code):`n$($r.Text)"
    $want = '#1=killed #2=no build #3=killed #4=killed #5=survived #6=survived'
    $got = @(foreach ($l in $r.Lines) { if ($l -match '^\s+(#\d+)\s+line\s+\d+\s+.*: (killed|survived|no build|timed out)$') { "$($Matches[1])=$($Matches[2])" } }) -join ' '
    Check ($got -eq $want) "normal run: each edit's result: expected '$want', got '$got':`n$($r.Text)"
    Check ($r.Text -match 'Score: 3 of 5 killed \(60%\)\.') "normal run: no 'Score: 3 of 5 killed (60%).' line:`n$($r.Text)"
    Check ($r.Text -match '1 more edit\(s\) did not compile') "normal run: the no-build edit is not reported:`n$($r.Text)"
    Check ($r.Text -match '(?m)^  line 7: `return true` became `return false`$' -and $r.Text -match '(?m)^  line 8: `return false` became `return true`$') "normal run: the two survivors are not listed:`n$($r.Text)"
    & $restored 'normal run'

    # -Only through pwsh -File, which hands "1,5" over as one string.
    $r = Invoke-Probe $wt $runner @('-Only', '1,5')
    $got = @(foreach ($l in $r.Lines) { if ($l -match '^\s+(#\d+)\s+line\s+\d+\s+.*: (killed|survived|no build|timed out)$') { "$($Matches[1])=$($Matches[2])" } }) -join ' '
    Check ($r.Code -eq 0 -and $got -eq '#1=killed #5=survived') "-Only 1,5: expected '#1=killed #5=survived', got exit $($r.Code) and '$got':`n$($r.Text)"
    & $restored '-Only run'

    # A run that breaks part way: the second build deletes the test program.
    $runner2 = Join-Path $fixture 'fake_tests_2.ps1'
    Copy-Item $runner $runner2
    $env:MUTPROBE_FAKE_CRASH = $runner2
    try { $r = Invoke-Probe $wt $runner2 } finally { Remove-Item Env:MUTPROBE_FAKE_CRASH }
    Check ($r.Code -eq 1 -and $r.Text -match 'test program is missing') "broken run: expected exit 1 and 'test program is missing', got exit $($r.Code):`n$($r.Text)"
    & $restored 'broken run'
} finally {
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_mutation_probe: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_mutation_probe: PASS, $($script:Passes) checks"
exit 0
