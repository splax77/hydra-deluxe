#Requires -Version 7.0
<#
The harm guard for the agent effort test. The main session runs it before and
after every live step of that test.

    pwsh -NoProfile -File tools/effort_test_guard.ps1
    pwsh -NoProfile -File tools/effort_test_guard.ps1 -Record

It prints one line for each of four checks and starts each with PASS or FAIL.
It exits 0 when all four pass and 1 otherwise.

The effort line says whether CLAUDE_CODE_EFFORT_LEVEL is empty, because that
variable would override every per-call effort level. The version line compares
`claude --version` with the version file. With -Record it writes the file
instead, and says "recorded". The branches line says whether a test branch has
reached main. The worktrees line says whether every test branch's worktree sits
directly in the main checkout's .claude\worktrees folder, the one place the
delete rule trusts.

$TestBranchPrefix below owns what counts as a test branch. A branch that holds
the test's own tools or results does not count.

-VersionFile and -Repo exist for tools/test_effort_test_guard.ps1 and for the
plan's run with a temporary version file. -Repo may name any checkout of the
repository.
#>
[CmdletBinding()]
param(
    [switch]$Record,
    [string]$VersionFile = 'C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\effort-test-results\docs\superpowers\specs\2026-11-07-agent-effort-test\claude-version.txt',
    [string]$Repo = (Split-Path $PSScriptRoot -Parent)
)

$ErrorActionPreference = 'Stop'
$TestBranchPrefix = 'claude/eff-'
$script:AnyFailed = $false

function Write-Check([string]$Label, [scriptblock]$Check) {
    # A check returns its passing text and throws its failing text, so a git
    # error is a FAIL line and the other checks still print theirs.
    try {
        Write-Host "PASS ${Label}: $(& $Check)"
    } catch {
        $script:AnyFailed = $true
        Write-Host "FAIL ${Label}: $($_.Exception.Message)"
    }
}

function Invoke-Git([string[]]$GitArgs) {
    $out = @(& git -C $Repo @GitArgs 2>&1 | ForEach-Object { "$_" })
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' ') failed: $($out -join ' ')" }
    $out
}

function Test-TestBranch([string]$Name) {
    $Name.StartsWith($TestBranchPrefix, [StringComparison]::Ordinal)
}

function Get-ClaudeVersion {
    $out = (@(& claude --version 2>&1 | ForEach-Object { "$_" }) -join ' ').Trim()
    if ($LASTEXITCODE -ne 0) { throw "claude --version failed: $out" }
    # Refuses any other text, so -Record never stores an error message.
    if ($out -notmatch '^\d+(\.\d+)+(?=\s|$)') { throw "claude --version printed no version: $out" }
    $Matches[0]
}

function Test-EffortLevel {
    $level = $env:CLAUDE_CODE_EFFORT_LEVEL
    if ($level) { throw "CLAUDE_CODE_EFFORT_LEVEL is '$level'" }
    'CLAUDE_CODE_EFFORT_LEVEL is empty'
}

function Test-ClaudeVersion {
    $running = Get-ClaudeVersion
    if ($Record) {
        # A missing folder is a failure, not something to create: a folder
        # made here would block the results worktree from being added.
        $folder = Split-Path $VersionFile -Parent
        if (-not (Test-Path -LiteralPath $folder)) { throw "the folder for the version file is missing: $folder" }
        [IO.File]::WriteAllText($VersionFile, "$running`n")
        return "recorded $running in $VersionFile"
    }
    if (-not (Test-Path -LiteralPath $VersionFile)) { throw "no version file at $VersionFile" }
    $expected = [IO.File]::ReadAllText($VersionFile).Trim()
    if ($running -cne $expected) { throw "running $running, but $VersionFile holds '$expected'" }
    "$running matches $VersionFile"
}

function Test-Branches {
    # Without this, a repository with no main would look clean below.
    $null = Invoke-Git @('rev-parse', '--verify', '--quiet', 'refs/heads/main')
    $names = @(Invoke-Git @('for-each-ref', '--format=%(refname:lstrip=2)', 'refs/heads/claude/') |
        Where-Object { Test-TestBranch $_ })
    $inMain = @($names | Where-Object {
        @(Invoke-Git @('branch', '--format=%(refname:lstrip=2)', '--contains', "refs/heads/$_", '--list', 'main')).Count -gt 0
    })
    if ($inMain.Count) { throw "in main: $($inMain -join ', ')" }
    "none of the $($names.Count) test branches is in main"
}

function Test-Worktrees {
    $lines = Invoke-Git @('worktree', 'list', '--porcelain')
    # Git lists the main checkout first, whichever checkout -Repo names.
    $mainCheckout = ($lines | Where-Object { $_.StartsWith('worktree ') } | Select-Object -First 1).Substring(9)
    $root = [IO.Path]::GetFullPath((Join-Path $mainCheckout '.claude\worktrees'))
    $checked = 0
    $outside = [System.Collections.Generic.List[string]]::new()
    $path = ''
    foreach ($line in $lines) {
        if ($line.StartsWith('worktree ')) {
            $path = [IO.Path]::GetFullPath($line.Substring(9))
        } elseif ($line.StartsWith('branch refs/heads/') -and (Test-TestBranch $line.Substring(18))) {
            $checked++
            if ((Split-Path $path -Parent) -ine $root) { $outside.Add($path) }
        }
    }
    if ($outside.Count) { throw "not directly in ${root}: $($outside -join ', ')" }
    "all $checked test worktrees sit directly in $root"
}

Write-Check 'effort' { Test-EffortLevel }
Write-Check 'version' { Test-ClaudeVersion }
Write-Check 'branches' { Test-Branches }
Write-Check 'worktrees' { Test-Worktrees }

exit ([int]$script:AnyFailed)
