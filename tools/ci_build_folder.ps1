#Requires -Version 7.0
<#
Lets a CI run start from the build folder an earlier run saved in GitHub's
cache, so a push recompiles only what it changed.

    pwsh -NoProfile -File tools/ci_build_folder.ps1 -Prepare [-BuildDir build-cpp]
    pwsh -NoProfile -File tools/ci_build_folder.ps1 -Record  [-BuildDir build-cpp]

-Prepare runs after the cache restore and before the build. A fresh checkout
gives every file the current time, so every object would look out of date.
-Prepare decides whether the restored folder can be used. If it can, it
gives every tracked file one old fixed time and gives "now" only to the files
changed since the commit the folder was built from, so the build recompiles
just those. If it can't, it deletes the folder and the build starts cold.
Which case applies is decided in Invoke-Prepare below; it prints one line
saying which path it took and why.

-Record runs after the build. It writes the commit just built into the
folder, for the next run's -Prepare to read.

The build folder is relative to -Repo, which defaults to this script's
checkout. Any git error stops the script with a message; it never falls
through to a warm build. The plan is
docs/superpowers/plans/2026-10-24-ci-build-speed.md.
#>
[CmdletBinding()]
param(
    [switch]$Prepare,
    [switch]$Record,
    [string]$BuildDir = 'build-cpp',
    # The checkout to work in. The self-test points it at a fixture repo.
    [string]$Repo = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'

# The files whose change makes the saved folder unsafe to reuse: a change to
# any of them starts the build cold. An entry ending in "/" covers its whole
# folder. Regenerating projects in place is MSBuild's riskiest incremental
# path, and these files change rarely, so a cold build costs little.
$script:BuildFiles = @(
    'CMakeLists.txt',
    'CMakePresets.json',
    'build_cpp.ps1',
    'tools/find_cmake.ps1',
    'tools/cmake_presets.ps1',
    'third_party/'
)

# The time every unchanged tracked file gets: older than any saved object.
$script:OldTime = [datetime]::new(2000, 1, 1, 0, 0, 0, [DateTimeKind]::Utc)

$RecordName = '.ci-built-commit'

function Say([string]$Line) { Write-Host "ci_build_folder: $Line" }

# Runs git in the repo. Returns the output lines; throws on a non-zero exit
# unless that exit code is listed in -Allow.
function Invoke-Git([string[]]$GitArgs, [int[]]$Allow = @()) {
    $all = @(& git -C $Repo -c core.quotepath=off @GitArgs 2>&1)
    $code = $LASTEXITCODE
    if ($code -ne 0 -and $Allow -notcontains $code) {
        throw "git $($GitArgs -join ' ') failed (exit $code): $(($all | Out-String).Trim())"
    }
    # Only stdout is data; git's warnings on stderr must not read as paths.
    $stdout = @($all | Where-Object { $_ -isnot [System.Management.Automation.ErrorRecord] } | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $code; Lines = $stdout }
}

# Splits git's -z output (one string with NUL separators) into paths.
function Split-Nul([string[]]$Lines) {
    @(($Lines -join "`n") -split "`0" | ForEach-Object { $_.Trim("`n") } | Where-Object { $_ -ne '' })
}

function Test-BuildFile([string]$Path) {
    foreach ($b in $script:BuildFiles) {
        if ($b.EndsWith('/')) { if ($Path.StartsWith($b)) { return $true } }
        elseif ($Path -eq $b) { return $true }
    }
    $false
}

function Remove-BuildFolder([string]$Dir, [string]$Why) {
    Remove-Item -Recurse -Force -LiteralPath $Dir
    Say "cold build: deleted $BuildDir because $Why."
}

function Invoke-Prepare([string]$Dir) {
    $recordPath = Join-Path $Dir $RecordName
    if (-not (Test-Path -LiteralPath $Dir -PathType Container)) {
        Say "cold build: no saved $BuildDir folder was restored."
        return
    }
    if (-not (Test-Path -LiteralPath $recordPath -PathType Leaf)) {
        Say "cold build: $BuildDir has no $RecordName record, so it was left as it is."
        return
    }
    $recorded = ([IO.File]::ReadAllText($recordPath)).Trim()
    if ($recorded -notmatch '^[0-9a-f]{40}$') {
        throw "$recordPath does not hold a full commit id (it holds '$recorded')."
    }

    # A commit missing from the clone can't be an ancestor of HEAD: it was
    # left behind by a force push or a rewritten branch.
    $exists = Invoke-Git @('cat-file', '-e', "$recorded^{commit}") -Allow @(1, 128)
    if ($exists.Code -ne 0) {
        Remove-BuildFolder $Dir "its commit $($recorded.Substring(0, 12)) is not in this clone"
        return
    }
    $anc = Invoke-Git @('merge-base', '--is-ancestor', $recorded, 'HEAD') -Allow @(1)
    if ($anc.Code -eq 1) {
        Remove-BuildFolder $Dir "its commit $($recorded.Substring(0, 12)) is not an ancestor of HEAD"
        return
    }

    $changed = Split-Nul (Invoke-Git @('diff', '--name-only', '--no-renames', '-z', $recorded, 'HEAD')).Lines
    $build = @($changed | Where-Object { Test-BuildFile $_ })
    if ($build.Count -gt 0) {
        Remove-BuildFolder $Dir "a build file changed since $($recorded.Substring(0, 12)) ($($build -join ', '))"
        return
    }

    $tracked = Split-Nul (Invoke-Git @('ls-files', '-z')).Lines
    $root = (Resolve-Path -LiteralPath $Repo).Path
    foreach ($f in $tracked) {
        $p = Join-Path $root $f
        if (Test-Path -LiteralPath $p -PathType Leaf) { [IO.File]::SetLastWriteTimeUtc($p, $script:OldTime) }
    }
    $now = [datetime]::UtcNow
    $touched = 0
    foreach ($f in $changed) {
        $p = Join-Path $root $f
        if (Test-Path -LiteralPath $p -PathType Leaf) { [IO.File]::SetLastWriteTimeUtc($p, $now); $touched++ }
    }
    Say "warm build: reusing $BuildDir built at $($recorded.Substring(0, 12)); $touched of $($tracked.Count) tracked files changed since then and were marked new."
}

function Invoke-Record([string]$Dir) {
    if (-not (Test-Path -LiteralPath $Dir -PathType Container)) {
        throw "there is no $BuildDir folder to record a commit in."
    }
    $head = (Invoke-Git @('rev-parse', '--verify', 'HEAD^{commit}')).Lines[0].Trim()
    [IO.File]::WriteAllText((Join-Path $Dir $RecordName), "$head`n")
    Say "recorded $head in $BuildDir\$RecordName."
}

if ($Prepare -eq $Record) { throw 'give exactly one of -Prepare or -Record.' }
if (-not (Test-Path -LiteralPath $Repo -PathType Container)) { throw "the repo folder $Repo does not exist." }
$repoFull = [IO.Path]::GetFullPath($Repo).TrimEnd('\', '/')
$dirFull = [IO.Path]::GetFullPath((Join-Path $repoFull $BuildDir)).TrimEnd('\', '/')
# The folder may be deleted, so it must sit strictly inside the repo.
if (-not $dirFull.StartsWith($repoFull + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw "-BuildDir $BuildDir must name a folder inside $repoFull."
}

if ($Prepare) { Invoke-Prepare $dirFull } else { Invoke-Record $dirFull }
