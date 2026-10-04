# Make a task worktree and build it, at most three builds at a time.
#
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p35-c2
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p3-k2 -Base claude/p3-k1 -Target hydra_tests
#
# It makes <main checkout>\.claude\worktrees\<TaskId> on a new branch
# claude/<TaskId> from -Base (default main), then builds -Target (default
# hydra_tests) there from nothing with build_cpp.ps1.
#
# Before the build it takes one of three machine-wide build slots, the same
# lock files phase 3's cold_build.ps1 uses (C:\Users\Patrick\.claude\hooks\
# state\build_slots\slot1..3.lock, each holding the owner's process id and
# worktree). A slot whose owner process is gone is taken over. While all three
# are busy it waits in the foreground and prints a line each minute. The slot
# is released when the build ends, pass or fail.
#
# Why cold builds and not a copied build folder (P0-4, measured 2026-10-04 on
# main 876b507, timing configure plus `hydra_tests`, no other compiler running
# at the start of either run):
#   Cold: configure 26.2 s + build 72.6 s = 98.8 s (0 compiler processes at
#     the start, 0 at the end).
#   Seeded from the main checkout's build-cpp: copy 0.4 s + re-point 5.9 s +
#     configure 0.6 s + tracking-log rename 0.4 s + build 48.5 s = 55.8 s
#     (0 compiler processes at the start, 8 at the end). 119 files recompiled.
# Seeding had to save at least half (49.4 s or less) to be worth its risk. It
# saved 44 percent. Two things eat the saving. The main checkout's build-cpp
# trails main (other sessions build only some targets there), so most objects
# were stale anyway. And every object that bakes in a path #define
# (HYDRA_TESTDATA_DIR and friends) has to recompile in a new tree: 41 objects,
# even when the copy comes from a fully built tree at the same commit (that
# best case took 27.0 s). Seeding also needs surgery on MSBuild's tracking
# logs, whose mistakes are silent wrong binaries. So this helper only staggers.

param(
    [Parameter(Mandatory = $true)][ValidatePattern('^[A-Za-z0-9._-]+$')][string]$TaskId,
    [string]$Base = "main",
    [string]$Target = "hydra_tests"
)

$ErrorActionPreference = "Stop"

# The main checkout owns .claude\worktrees, wherever this script is run from.
$common = git -C $PSScriptRoot rev-parse --path-format=absolute --git-common-dir
if ($LASTEXITCODE -ne 0) { throw "not inside a git checkout: $PSScriptRoot" }
$main = Split-Path ([IO.Path]::GetFullPath($common))
$wt = Join-Path $main ".claude\worktrees\$TaskId"
$branch = "claude/$TaskId"

if (Test-Path $wt) { throw "worktree folder already exists: $wt" }
git -C $main rev-parse --verify --quiet "refs/heads/$branch" | Out-Null
if ($LASTEXITCODE -eq 0) { throw "branch already exists: $branch" }

git -C $main worktree add $wt -b $branch $Base
if ($LASTEXITCODE -ne 0) { throw "git worktree add failed for $wt" }

# One of three machine-wide build slots (shared with phase 3's cold_build.ps1).
$slots = "C:\Users\Patrick\.claude\hooks\state\build_slots"
New-Item -ItemType Directory -Force $slots | Out-Null
$mine = $null
$waited = 0
while (-not $mine) {
    foreach ($i in 1..3) {
        $f = Join-Path $slots "slot$i.lock"
        if (Test-Path $f) {
            $owner = (Get-Content $f -ErrorAction SilentlyContinue | Select-Object -First 1) -as [int]
            if ($owner -and (Get-Process -Id $owner -ErrorAction SilentlyContinue)) { continue }
            Remove-Item $f -Force -ErrorAction SilentlyContinue
        }
        try {
            $fs = [System.IO.File]::Open($f, 'CreateNew', 'Write')
            $b = [System.Text.Encoding]::ASCII.GetBytes("$PID`n$wt")
            $fs.Write($b, 0, $b.Length); $fs.Close()
            $mine = $f; break
        } catch { }
    }
    if (-not $mine) {
        if ($waited % 60 -eq 0) { Write-Host "waiting for a build slot ($waited s)" }
        Start-Sleep -Seconds 15; $waited += 15
    }
}
Write-Host "build slot $mine taken"

$failed = $null
$clock = [Diagnostics.Stopwatch]::StartNew()
try {
    & (Join-Path $wt "build_cpp.ps1") -Target $Target
} catch {
    $failed = $_
} finally {
    Remove-Item $mine -Force -ErrorAction SilentlyContinue
}
$secs = [int]$clock.Elapsed.TotalSeconds
if ($failed) {
    Write-Host "build of $Target failed in $wt after $secs s: $failed"
    exit 1
}
Write-Host "worktree $wt on $branch, $Target built in $secs s"
