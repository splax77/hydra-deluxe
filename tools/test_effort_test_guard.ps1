#Requires -Version 7.0
<#
Self-test for tools/effort_test_guard.ps1.

    pwsh -NoProfile -File tools/test_effort_test_guard.ps1 [-Scratch <folder>]

It builds a tiny git repository and a stand-in `claude` command in a new
folder inside -Scratch (the system temp folder by default), then runs the real
guard on them through -Repo and -VersionFile, in its own pwsh each time. The
fixture is deleted at the end; this repository is never touched.
#>
[CmdletBinding()]
param(
    [string]$Scratch = [IO.Path]::GetTempPath()
)

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$tool = Join-Path $PSScriptRoot 'effort_test_guard.ps1'

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

$fixture = Join-Path $Scratch ("effort_guard_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
$repo = Join-Path $fixture 'repo'
$bin = Join-Path $fixture 'bin'
$versionFolder = Join-Path $fixture 'spec'
$versionFile = Join-Path $versionFolder 'claude-version.txt'
$worktrees = Join-Path $repo '.claude\worktrees'
New-Item -ItemType Directory -Path $repo, $bin, $versionFolder | Out-Null

function Invoke-FixtureGit([string[]]$GitArgs, [string]$In = $repo) {
    $out = & git -C $In -c user.name=probe -c user.email=probe@example.invalid -c core.autocrlf=false @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' '): $($out | Out-String)" }
    $out
}
function Add-Commit([string]$File) {
    [IO.File]::WriteAllText((Join-Path $repo $File), "$File`n")
    Invoke-FixtureGit @('add', $File) | Out-Null
    Invoke-FixtureGit @('commit', '-q', '-m', $File) | Out-Null
}
# Makes a branch with one commit of its own and merges it into main.
function Add-MergedBranch([string]$Name, [string]$File) {
    Invoke-FixtureGit @('checkout', '-q', '-b', $Name) | Out-Null
    Add-Commit $File
    Invoke-FixtureGit @('checkout', '-q', 'main') | Out-Null
    Invoke-FixtureGit @('merge', '-q', '--no-ff', '-m', "merge $Name", $Name) | Out-Null
}
function Set-StandIn([string]$Lines) {
    [IO.File]::WriteAllText((Join-Path $bin 'claude.cmd'), "@echo off`r`n$Lines`r`n")
}
function Set-Version([string]$Text) { [IO.File]::WriteAllText($versionFile, $Text) }
function Invoke-Guard([string[]]$Extra = @(), [string]$Target = $repo, [string]$File = $versionFile) {
    $lines = @(& pwsh -NoProfile -File $tool -Repo $Target -VersionFile $File @Extra 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $LASTEXITCODE; Lines = $lines; Text = ($lines -join "`n") }
}
# The one line a check printed, or '' when it printed none or several.
function Line-Of($Result, [string]$Label) {
    $found = @($Result.Lines | Where-Object { $_ -match "^(PASS|FAIL) ${Label}:" })
    if ($found.Count -eq 1) { $found[0] } else { '' }
}
# Holds a run to its exit code and to the checks that must fail; every other
# check must pass, and each prints exactly one line.
function Check-Run([string]$Case, $Result, [int]$Code, [string[]]$Failing = @()) {
    Check ($Result.Code -eq $Code) "${Case}: exit $($Result.Code), wanted ${Code}:`n$($Result.Text)"
    Check ($Result.Lines.Count -eq 4) "${Case}: $($Result.Lines.Count) lines, wanted 4:`n$($Result.Text)"
    foreach ($label in 'effort', 'version', 'branches', 'worktrees') {
        $word = if ($label -in $Failing) { 'FAIL' } else { 'PASS' }
        Check ((Line-Of $Result $label).StartsWith("$word $label")) "${Case}: wanted $word on '$label':`n$($Result.Text)"
    }
}

$savedPath = $env:Path
$savedEffort = $env:CLAUDE_CODE_EFFORT_LEVEL
try {
    $env:Path = "$bin;$env:Path"
    $env:CLAUDE_CODE_EFFORT_LEVEL = $null
    Set-StandIn 'echo 9.9.9 (Claude Code)'
    Set-Version "9.9.9`n"
    Invoke-FixtureGit @('init', '-q', '-b', 'main') | Out-Null
    Add-Commit 'first.txt'

    # A clean repository passes all four checks.
    Check-Run 'clean' (Invoke-Guard) 0

    # The effort variable.
    $env:CLAUDE_CODE_EFFORT_LEVEL = 'medium'
    $r = Invoke-Guard
    Check-Run 'effort set' $r 1 @('effort')
    Check ((Line-Of $r 'effort') -match 'medium') "effort set: the line does not name the level:`n$($r.Text)"
    $env:CLAUDE_CODE_EFFORT_LEVEL = $null

    # The version file: another version, and no file.
    Set-Version "9.9.8`n"
    $r = Invoke-Guard
    Check-Run 'other version' $r 1 @('version')
    Check ((Line-Of $r 'version') -match '9\.9\.9.*9\.9\.8') "other version: the line does not name both:`n$($r.Text)"
    Remove-Item -LiteralPath $versionFile
    Check-Run 'no version file' (Invoke-Guard) 1 @('version')

    # -Record writes the running version, and the next plain run passes.
    $r = Invoke-Guard @('-Record')
    Check-Run 'record' $r 0
    Check ((Line-Of $r 'version') -match 'recorded') "record: no 'recorded' on the line:`n$($r.Text)"
    Check ([IO.File]::ReadAllText($versionFile).Trim() -ceq '9.9.9') "record: the file holds '$([IO.File]::ReadAllText($versionFile))'"
    Check-Run 'after record' (Invoke-Guard) 0

    # -Record fails loudly on a missing folder and leaves it missing.
    $noFolder = Join-Path $fixture 'absent\claude-version.txt'
    Check-Run 'record, no folder' (Invoke-Guard @('-Record') $repo $noFolder) 1 @('version')
    Check (-not (Test-Path (Split-Path $noFolder))) 'record, no folder: the folder was created'

    # A claude that fails, or prints no version, never lands in the file.
    Set-StandIn "echo something broke`r`nexit /b 3"
    Check-Run 'claude fails' (Invoke-Guard @('-Record')) 1 @('version')
    Set-StandIn 'echo Error: not logged in'
    Check-Run 'claude prints no version' (Invoke-Guard @('-Record')) 1 @('version')
    Check ([IO.File]::ReadAllText($versionFile).Trim() -ceq '9.9.9') 'bad claude output: the version file changed'
    Set-StandIn 'echo 9.9.9 (Claude Code)'

    # The test's own tools and results branches are not test branches, even
    # merged into main and with a worktree outside the trusted folder.
    Add-MergedBranch 'claude/effort-test-tools' 'tools.txt'
    Invoke-FixtureGit @('worktree', 'add', '-q', (Join-Path $fixture 'tools-wt'), '-b', 'claude/effort-test-results') | Out-Null
    $r = Invoke-Guard
    Check-Run 'claude/effort-*' $r 0
    Check ((Line-Of $r 'branches') -match '\b0 test branches') "claude/effort-*: counted as a test branch:`n$($r.Text)"
    Check ((Line-Of $r 'worktrees') -match '\b0 test worktrees') "claude/effort-*: counted as a test worktree:`n$($r.Text)"

    # An unmerged test branch in the right place passes, and is counted.
    Invoke-FixtureGit @('worktree', 'add', '-q', (Join-Path $worktrees 'eff-good'), '-b', 'claude/eff-good') | Out-Null
    Invoke-FixtureGit @('commit', '-q', '--allow-empty', '-m', 'run') (Join-Path $worktrees 'eff-good') | Out-Null
    $r = Invoke-Guard
    Check-Run 'unmerged test branch' $r 0
    Check ((Line-Of $r 'branches') -match '\b1 test branches') "unmerged test branch: not counted:`n$($r.Text)"
    Check ((Line-Of $r 'worktrees') -match '\b1 test worktrees') "unmerged test worktree: not counted:`n$($r.Text)"

    # -Repo naming a linked worktree still finds the main checkout's folder.
    Check-Run 'repo is a worktree' (Invoke-Guard @() (Join-Path $worktrees 'eff-good')) 0

    # The plan's acceptance case: claude/eff-x merged into main. A name with
    # a further slash is caught too, and the tools branch is still not named.
    Add-MergedBranch 'claude/eff-x' 'x.txt'
    Add-MergedBranch 'claude/eff-deep/y' 'y.txt'
    $r = Invoke-Guard
    Check-Run 'merged test branch' $r 1 @('branches')
    $line = Line-Of $r 'branches'
    Check ($line -match 'claude/eff-x\b') "merged test branch: claude/eff-x not named:`n$($r.Text)"
    Check ($line -match 'claude/eff-deep/y\b') "merged test branch: claude/eff-deep/y not named:`n$($r.Text)"
    Check ($line -notmatch 'effort-test-tools|eff-good') "merged test branch: the wrong branch named:`n$($r.Text)"
    $r = Invoke-Guard @('-Record')
    Check-Run 'record, merged test branch' $r 1 @('branches')
    Check ((Line-Of $r 'version') -match 'recorded') "record, merged test branch: no 'recorded':`n$($r.Text)"
    Invoke-FixtureGit @('branch', '-q', '-D', 'claude/eff-x', 'claude/eff-deep/y') | Out-Null
    Check-Run 'merged branches gone' (Invoke-Guard) 0

    # Worktrees one level too deep and outside the folder fail; the good one
    # stays unnamed.
    $nested = Join-Path $worktrees 'nest\eff-nested'
    $far = Join-Path $fixture 'eff-far'
    Invoke-FixtureGit @('worktree', 'add', '-q', $nested, '-b', 'claude/eff-nested') | Out-Null
    Invoke-FixtureGit @('worktree', 'add', '-q', $far, '-b', 'claude/eff-far') | Out-Null
    Invoke-FixtureGit @('commit', '-q', '--allow-empty', '-m', 'run') $nested | Out-Null
    Invoke-FixtureGit @('commit', '-q', '--allow-empty', '-m', 'run') $far | Out-Null
    $r = Invoke-Guard
    Check-Run 'misplaced worktrees' $r 1 @('worktrees')
    $line = Line-Of $r 'worktrees'
    Check ($line.Contains($nested) -and $line.Contains($far)) "misplaced worktrees: not both named:`n$($r.Text)"
    Check (-not $line.Contains('eff-good')) "misplaced worktrees: the good one named:`n$($r.Text)"

    # A repository with no main fails rather than looking clean.
    $trunk = Join-Path $fixture 'trunk'
    New-Item -ItemType Directory -Path $trunk | Out-Null
    Invoke-FixtureGit @('init', '-q', '-b', 'trunk') $trunk | Out-Null
    Invoke-FixtureGit @('commit', '-q', '--allow-empty', '-m', 'first') $trunk | Out-Null
    Check-Run 'no main' (Invoke-Guard @() $trunk) 1 @('branches')

    # A folder that is no repository fails both git checks, one line each.
    Check-Run 'not a repository' (Invoke-Guard @() $bin) 1 @('branches', 'worktrees')
} finally {
    $env:Path = $savedPath
    $env:CLAUDE_CODE_EFFORT_LEVEL = $savedEffort
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_effort_test_guard: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_effort_test_guard: PASS, $($script:Passes) checks"
exit 0
