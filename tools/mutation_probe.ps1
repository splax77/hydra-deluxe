#Requires -Version 7.0
<#
Mutation probe: does a test notice when this code breaks?

It plants one small deliberate bug at a time in one source file, rebuilds
hydra_tests, and runs only the tests you name. A bug that makes a test fail
is "killed". A bug no test notices is a "survivor": a spot the tests do not
guard, where a person can add a test. Think of it as a fire drill for the
tests: set off one alarm at a time and see whether anyone comes.

    pwsh -NoProfile -File tools\mutation_probe.ps1 -Source src\core\stars.cpp -Filter sf=*test_stars* -Max 20
    pwsh -NoProfile -File tools\mutation_probe.ps1 -Source src\core\stars.cpp -List
    pwsh -NoProfile -File tools\mutation_probe.ps1 -Source src\core\stars.cpp -Filter sf=*test_stars* -Only 4,9

-Filter is one doctest filter for hydra_tests, -tc= (test case) or -sf=
(source file). Write it without the leading dash (sf=*test_stars*), because
pwsh -File reads a dash-led argument as a parameter name; the dash is added
back here. -List prints the numbered candidates and builds nothing. -Only
reruns chosen candidate numbers from -List, for instance after adding a test
for a survivor. With more candidates than -Max, an even spread through the
file is tried (Select-Spread).

The kinds of edit it makes are Find-MutationCandidates' list below. Edits go
only into code: Get-CodeMask masks the rest first. Comments and string text
are the precheck's answer (tools\derive_once_precheck.ps1's Remove-Comments,
loaded from its text, not copied); the probe adds preprocessor lines.

Each run:
  - refuses the main checkout, because it edits source; run it in a task
    worktree (tools\new_worktree.ps1 makes one);
  - refuses a source file that differs from git, so "restored" means "matches
    git";
  - holds one machine build slot for the whole run (tools\build_slot.ps1
    owns the slot rule; its functions are loaded from its text, not copied);
  - builds and tests the unmutated file first, and stops if that fails or
    the filter matches no test case;
  - restores the file after every edit and again at the end, in finally
    blocks, so an error or Ctrl-C still puts it back; then checks it matches
    git and exits 2, loudly, if it does not;
  - rebuilds the unmutated file at the end, so the worktree's hydra_tests.exe
    matches its source again.

The report (printed, and saved as report.txt in -OutDir with every build and
test log) gives the score, killed out of the edits that built, and each
survivor's line and edit. An edit that does not compile is "no build" and
left out of the score. A test run that outlives the time limit
(Get-TestTimeoutMs) is "timed out": most likely the edit made a loop endless,
so it counts as killed, but it is listed so a person can check.

-TestExe and -SlotDir exist for tools\test_mutation_probe.ps1, which runs
the probe on a small fixture with a fake build and a fake test program and
must never take a real build slot. A -TestExe ending in .ps1 runs under pwsh.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Source,
    [string]$Filter = '',
    [int]$Max = 0,
    # Candidate numbers. A string, split here: pwsh -File would read "4,9" as the number 49.
    [string[]]$Only = @(),
    [switch]$List,
    # The worktree to run in; default: the checkout holding this script.
    [string]$Repo = '',
    # Where the report and logs go; default: a new folder under build-cpp\mutation_probe.
    [string]$OutDir = '',
    [string]$TestExe = '',
    [string]$SlotDir = ''
)

$ErrorActionPreference = 'Stop'

# ------------------------------------------------------------------ the finder

# The C++ lexer and its blanking, as definition text from the precheck
# (path given), so this probe never reads comments and strings its own way.
# The caller dot-sources the text; the self-test loads it the same way.
function Get-PrecheckLexer([string]$Precheck) {
    $ast = [System.Management.Automation.Language.Parser]::ParseFile($Precheck, [ref]$null, [ref]$null)
    $want = '$cppLex', 'Blank', 'Blank-Inner', 'Remove-Comments'
    $text = foreach ($name in $want) {
        $f = $ast.Find({ param($n)
                ($n -is [System.Management.Automation.Language.AssignmentStatementAst] -and "$($n.Left)" -eq $name) -or
                ($n -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $n.Name -eq $name) }, $true)
        if (-not $f) { throw "cannot find $name in $Precheck" }
        $f.Extent.Text
    }
    $text -join "`n"
}
. ([scriptblock]::Create((Get-PrecheckLexer (Join-Path $PSScriptRoot 'derive_once_precheck.ps1'))))

# The source text with everything that is not code turned into spaces. Line
# breaks stay, so offsets and line numbers match the source. Comments and
# string text are the precheck's Remove-Comments; preprocessor lines are this
# probe's own addition.
function Get-CodeMask([string]$Text) {
    $masked = Remove-Comments $cppLex $Text -AndStrings
    [regex]::Replace($masked, '(?m)^[ \t]*#(?:[^\n]*\\\r?\n)*[^\n]*', { param($m) Blank $m.Value })
}

# Every spot in the code this probe can break, in file order, numbered from
# 1. Each candidate is one edit: replace Length characters at Offset with
# Replacement. Was and Now say the edit in words for the report.
function Find-MutationCandidates([string]$Text) {
    $mask = Get-CodeMask $Text
    $found = [System.Collections.Generic.List[object]]::new()
    $add = { param($Offset, $Length, $Replacement, $Was, $Now)
        $found.Add([pscustomobject]@{ Offset = $Offset; Length = $Length; Replacement = $Replacement; Was = $Was; Now = $Now }) }

    # Comparison flips, each way. Spaces on both sides tell a comparison from
    # a template bracket, a shift or an arrow.
    $flip = @{ '<' = '<='; '<=' = '<'; '>' = '>='; '>=' = '>'; '==' = '!='; '!=' = '==' }
    foreach ($m in [regex]::Matches($mask, '(?<=\s)(?:<=|>=|==|!=|<|>)(?=\s)')) {
        & $add $m.Index $m.Length $flip[$m.Value] "``$($m.Value)``" "``$($flip[$m.Value])``"
    }
    # And becomes or.
    foreach ($m in [regex]::Matches($mask, '(?<=\s)&&(?=\s)')) {
        & $add $m.Index 2 '||' '`&&`' '`||`'
    }
    # A binary "+ 1" becomes "- 1", or goes. Not after "return" or "case"
    # (a unary plus), not an exponent (1e+1), not 1.5 or 10.
    $plusOne = '(?<=[\w\)\]])(?<!\b(?:return|case))(?<!\b\d[\d.]*[eE])(\s*)\+\s*1(?![\w.''])'
    foreach ($m in [regex]::Matches($mask, $plusOne)) {
        $said = $Text.Substring($m.Index, $m.Length).Trim()
        & $add $m.Index $m.Length '' "``$said``" 'nothing'
        & $add ($m.Index + $m.Groups[1].Length) 1 '-' "``$said``" "``$($said.Replace('+', '-'))``"
    }
    # A returned true or false flips.
    foreach ($m in [regex]::Matches($mask, '\breturn\s+(true|false)\s*;')) {
        $g = $m.Groups[1]
        $now = if ($g.Value -eq 'true') { 'false' } else { 'true' }
        & $add $g.Index $g.Length $now "``return $($g.Value)``" "``return $now``"
    }

    $n = 0
    foreach ($c in ($found | Sort-Object -Stable Offset)) {
        $n++
        $line = $Text.Substring(0, $c.Offset).Split("`n").Count
        $c | Add-Member Number $n
        $c | Add-Member Line $line
        $c
    }
}

# The source with one candidate's edit applied.
function Get-MutatedText([string]$Text, $Candidate) {
    $Text.Substring(0, $Candidate.Offset) + $Candidate.Replacement + $Text.Substring($Candidate.Offset + $Candidate.Length)
}

# The line holding a candidate's edit, after the edit, without its indent.
function Get-MutatedLine([string]$Text, $Candidate) {
    $mutated = Get-MutatedText $Text $Candidate
    ($mutated.Split("`n")[$Candidate.Line - 1]).Trim()
}

# Which candidate numbers a run tries: all of them up to Max, otherwise Max
# numbers spread evenly from the first.
function Select-Spread([int]$Count, [int]$Max) {
    if ($Count -le 0 -or $Max -le 0) { return @() }
    if ($Count -le $Max) { return @(1..$Count) }
    @(0..($Max - 1) | ForEach-Object { [int][Math]::Floor($_ * $Count / $Max) + 1 })
}

# How long a mutated test run may take before it counts as stuck. The user's
# decision is D81 item 3 in docs/audit/2026-10-03-fix-decisions.md: PIT's
# documented defaults (timeoutFactor, timeoutConst;
# pitest.org/quickstart/commandline), the JVM mutation tool's answer to the
# same question.
function Get-TestTimeoutMs([double]$BaselineSeconds) {
    [int]($BaselineSeconds * 1000 * 1.25 + 4000)
}

# True when two byte arrays hold the same bytes.
function Test-SameBytes([byte[]]$A, [byte[]]$B) {
    $A.Length -eq $B.Length -and [Convert]::ToBase64String($A) -eq [Convert]::ToBase64String($B)
}

# ------------------------------------------------------------------ the run

function Fail([string]$Why) { Write-Host "mutation_probe: $Why"; exit 1 }

if (-not $Repo) { $Repo = Join-Path $PSScriptRoot '..' }
$Repo = (& git -C $Repo rev-parse --show-toplevel 2>$null)
if ($LASTEXITCODE -ne 0 -or -not $Repo) { Fail "not inside a git checkout: $Repo" }
$Repo = [IO.Path]::GetFullPath($Repo)

$sourcePath = if ([IO.Path]::IsPathRooted($Source)) { $Source } else { Join-Path $Repo $Source }
$sourcePath = [IO.Path]::GetFullPath($sourcePath)
if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) { Fail "no such source file: $sourcePath" }
$rel = [IO.Path]::GetRelativePath($Repo, $sourcePath).Replace('\', '/')
if ($rel.StartsWith('..')) { Fail "$sourcePath is not inside $Repo" }

$originalBytes = [IO.File]::ReadAllBytes($sourcePath)
$utf8 = [Text.UTF8Encoding]::new($false)
$text = $utf8.GetString($originalBytes)
if (-not (Test-SameBytes $utf8.GetBytes($text) $originalBytes)) {
    Fail "$rel is not plain UTF-8, so an edit could not be undone byte for byte"
}
$candidates = @(Find-MutationCandidates $text)

if ($List) {
    Write-Host "$($candidates.Count) candidates in ${rel}:"
    foreach ($c in $candidates) { Write-Host ("  #{0,-3} line {1,-5} {2} to {3}    {4}" -f $c.Number, $c.Line, $c.Was, $c.Now, (Get-MutatedLine $text $c)) }
    exit 0
}

# The checks before anything is edited.
if ($Filter -notmatch '^-?(tc|sf)=.+') { Fail "give -Filter as tc=<test case> or sf=<source file>, got '$Filter'" }
if (-not $Filter.StartsWith('-')) { $Filter = "-$Filter" }
if ($Only.Count -eq 0 -and $Max -le 0) { Fail 'give -Max, the most edits to try (or -Only with candidate numbers from -List)' }
$gitDir = [IO.Path]::GetFullPath((& git -C $Repo rev-parse --absolute-git-dir)).TrimEnd('\', '/')
$commonDir = [IO.Path]::GetFullPath((& git -C $Repo rev-parse --path-format=absolute --git-common-dir)).TrimEnd('\', '/')
if ($gitDir -eq $commonDir) {
    Fail "$Repo is the main checkout. This probe edits source, so run it in a task worktree (tools\new_worktree.ps1)."
}
& git -C $Repo ls-files --error-unmatch -- $rel *> $null
if ($LASTEXITCODE -ne 0) { Fail "$rel is not tracked by git, so there is nothing to check the restore against" }
if (& git -C $Repo status --porcelain -- $rel) { Fail "$rel differs from git; commit or undo that first, so the restore can be checked" }
if (-not $TestExe) { $TestExe = Join-Path $Repo 'build-cpp\Release\hydra_tests.exe' }
$buildScript = Join-Path $Repo 'build_cpp.ps1'
if (-not (Test-Path -LiteralPath $buildScript)) { Fail "no build_cpp.ps1 in $Repo" }

$Only = @($Only -split ',' | ForEach-Object { $_.Trim() } | Where-Object { $_ })
foreach ($k in $Only) { if ($k -notmatch '^\d+$') { Fail "-Only takes candidate numbers, got '$k'" } }
$numbers = if ($Only.Count) { @($Only | ForEach-Object { [int]$_ }) } else { Select-Spread $candidates.Count $Max }
foreach ($k in $numbers) { if ($k -lt 1 -or $k -gt $candidates.Count) { Fail "no candidate #$k; -List shows 1 to $($candidates.Count)" } }
$chosen = @($numbers | ForEach-Object { $candidates[$_ - 1] })
if (-not $chosen.Count) { Fail "no candidates in $rel" }

if (-not $OutDir) {
    $OutDir = Join-Path $Repo ("build-cpp\mutation_probe\{0}-{1}" -f [IO.Path]::GetFileNameWithoutExtension($sourcePath), (Get-Date -Format 'yyyyMMdd-HHmmss'))
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$pwshPath = (Get-Process -Id $PID).Path

# The build slot functions and settings, loaded from build_slot.ps1's text.
$slotScript = Join-Path $PSScriptRoot 'build_slot.ps1'
$slotAst = [System.Management.Automation.Language.Parser]::ParseFile($slotScript, [ref]$null, [ref]$null)
$slotDefs = $slotAst.FindAll({ param($a)
        ($a -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $a.Name -in 'Get-BuildSlot', 'Release-BuildSlot') -or
        ($a -is [System.Management.Automation.Language.AssignmentStatementAst] -and $a.Parent -eq $slotAst.EndBlock -and "$($a.Left)" -like '$BuildSlot*') }, $false)
if (@($slotDefs | Where-Object { $_ -is [System.Management.Automation.Language.FunctionDefinitionAst] }).Count -ne 2) {
    Fail "cannot find Get-BuildSlot and Release-BuildSlot in $slotScript"
}
. ([scriptblock]::Create((@($slotDefs | ForEach-Object { $_.Extent.Text }) -join "`n")))

# One warm build of hydra_tests; true when it built.
function Invoke-Build([string]$Log) {
    & $pwshPath -NoProfile -File $buildScript -Target hydra_tests *> $Log
    $LASTEXITCODE -eq 0
}

# One filtered test run, killed after TimeoutMs (-1: no limit).
function Invoke-Tests([string]$Log, [int]$TimeoutMs) {
    if (-not (Test-Path -LiteralPath $TestExe)) { throw "the test program is missing: $TestExe" }
    $psi = [Diagnostics.ProcessStartInfo]::new()
    if ($TestExe -like '*.ps1') {
        $psi.FileName = $pwshPath
        foreach ($a in '-NoProfile', '-File', $TestExe) { $psi.ArgumentList.Add($a) }
    } else { $psi.FileName = $TestExe }
    $psi.ArgumentList.Add($Filter)
    $psi.WorkingDirectory = $Repo
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $clock = [Diagnostics.Stopwatch]::StartNew()
    $p = [Diagnostics.Process]::Start($psi)
    try {
        $out = $p.StandardOutput.ReadToEndAsync()
        $err = $p.StandardError.ReadToEndAsync()
        $timedOut = -not $p.WaitForExit($TimeoutMs)
        if ($timedOut) { $p.Kill($true); $p.WaitForExit() }
        $all = $out.Result + $err.Result
        [IO.File]::WriteAllText($Log, $all)
        $cases = if ($all -match 'test cases:\s*(\d+)') { [int]$Matches[1] } else { -1 }
        [pscustomobject]@{ Passed = (-not $timedOut -and $p.ExitCode -eq 0); TimedOut = $timedOut; Cases = $cases; Seconds = $clock.Elapsed.TotalSeconds }
    } finally {
        if (-not $p.HasExited) { $p.Kill($true) }
        $p.Dispose()
    }
}

# Puts the original bytes back when the file differs from them.
function Restore-Source {
    if (-not (Test-SameBytes ([IO.File]::ReadAllBytes($sourcePath)) $originalBytes)) {
        [IO.File]::WriteAllBytes($sourcePath, $originalBytes)
    }
}

$runClock = [Diagnostics.Stopwatch]::StartNew()
$started = Get-Date
$results = [System.Collections.Generic.List[object]]::new()
$slotArgs = @{ Owner = "mutation_probe $Repo" }
if ($SlotDir) { $slotArgs.Dir = $SlotDir }
$slot = $null
$baseline = $null
$exitCode = 0
try {
    $slot = Get-BuildSlot @slotArgs
    Write-Host "building the unmutated $rel"
    if (-not (Invoke-Build (Join-Path $OutDir 'baseline_build.log'))) { throw "the unmutated source does not build; see $OutDir\baseline_build.log" }
    $baseline = Invoke-Tests (Join-Path $OutDir 'baseline_tests.log') -1
    if (-not $baseline.Passed) { throw "the tests fail on the unmutated source; see $OutDir\baseline_tests.log" }
    if ($baseline.Cases -le 0) { throw "the filter $Filter matches no test case; see $OutDir\baseline_tests.log" }
    $timeoutMs = Get-TestTimeoutMs $baseline.Seconds
    Write-Host ("unmutated: {0} test cases pass in {1:0.0} s; a mutated run is stopped after {2:0.0} s" -f $baseline.Cases, $baseline.Seconds, ($timeoutMs / 1000))

    $i = 0
    foreach ($c in $chosen) {
        $i++
        $clock = [Diagnostics.Stopwatch]::StartNew()
        $status = $null
        try {
            [IO.File]::WriteAllText($sourcePath, (Get-MutatedText $text $c), $utf8)
            if (-not (Invoke-Build (Join-Path $OutDir "mutant_$($c.Number)_build.log"))) { $status = 'no build' }
            else {
                $t = Invoke-Tests (Join-Path $OutDir "mutant_$($c.Number)_tests.log") $timeoutMs
                $status = if ($t.TimedOut) { 'timed out' } elseif ($t.Passed) { 'survived' } else { 'killed' }
            }
        } finally {
            Restore-Source
        }
        $results.Add([pscustomobject]@{ Candidate = $c; Status = $status })
        Write-Host ("[{0}/{1}] #{2} line {3}: {4} to {5}: {6} ({7:0} s)" -f $i, $chosen.Count, $c.Number, $c.Line, $c.Was, $c.Now, $status, $clock.Elapsed.TotalSeconds)
    }

    Write-Host 'rebuilding the unmutated source, so hydra_tests.exe matches it again'
    if (-not (Invoke-Build (Join-Path $OutDir 'final_build.log'))) {
        Write-Host "mutation_probe: WARNING the final unmutated build failed; see $OutDir\final_build.log"
        $exitCode = 1
    }
} catch {
    Write-Host "mutation_probe: stopped: $_"
    $exitCode = 1
} finally {
    Restore-Source
    if ($slot) { Release-BuildSlot $slot }
    $same = Test-SameBytes ([IO.File]::ReadAllBytes($sourcePath)) $originalBytes
    $dirty = & git -C $Repo status --porcelain -- $rel
    if (-not $same -or $dirty) {
        Write-Host "mutation_probe: ERROR $rel does not match git after the run ($dirty). Restore it by hand: git -C $Repo checkout -- $rel"
        exit 2
    }
}

# ------------------------------------------------------------------ the report

$killed = @($results | Where-Object { $_.Status -in 'killed', 'timed out' })
$survived = @($results | Where-Object { $_.Status -eq 'survived' })
$noBuild = @($results | Where-Object { $_.Status -eq 'no build' })
$built = $killed.Count + $survived.Count
$elapsed = $runClock.Elapsed
$report = [System.Collections.Generic.List[string]]::new()
$report.Add('Mutation probe report')
$report.Add("File:  $rel")
if ($baseline) { $report.Add(("Tests: {0} ({1} test cases in the unmutated run, {2:0.0} s)" -f $Filter, $baseline.Cases, $baseline.Seconds)) }
$report.Add(("Run:   {0:yyyy-MM-dd HH:mm}, {1} of {2} candidates tried, {3} min {4} s" -f $started, $results.Count, $candidates.Count, [int][Math]::Floor($elapsed.TotalMinutes), $elapsed.Seconds))
if ($exitCode -ne 0 -and $results.Count -lt $chosen.Count) { $report.Add("The run stopped early; the results below are the edits it finished.") }
$report.Add('')
if ($built) {
    $report.Add(("Score: {0} of {1} killed ({2:0}%)." -f $killed.Count, $built, (100.0 * $killed.Count / $built)))
} else { $report.Add('Score: none of the edits built, so there is no score.') }
if ($noBuild.Count) { $report.Add("$($noBuild.Count) more edit(s) did not compile and are left out of the score.") }
$report.Add('')
if ($survived.Count) {
    $report.Add('Survivors. No test failed with these edits; each marks a spot a test could guard:')
    foreach ($r in $survived) {
        $c = $r.Candidate
        $report.Add("  line $($c.Line): $($c.Was) became $($c.Now)")
        $report.Add("      $(Get-MutatedLine $text $c)")
    }
} else { $report.Add('Survivors: none.') }
$report.Add('')
$report.Add('Every edit tried:')
foreach ($r in $results) {
    $c = $r.Candidate
    $report.Add(("  #{0,-3} line {1,-5} {2} to {3}: {4}" -f $c.Number, $c.Line, $c.Was, $c.Now, $r.Status))
}
$report.Add('')
$report.Add("Logs: $OutDir")
[IO.File]::WriteAllLines((Join-Path $OutDir 'report.txt'), $report)
$report | ForEach-Object { Write-Host $_ }
exit $exitCode
