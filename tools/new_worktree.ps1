# Make a task worktree and build it inside a machine-wide build slot.
#
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p35-c2
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p3-k2 -Base claude/p3-k1 -Target hydra_tests
#
# It makes <main checkout>\.claude\worktrees\<TaskId> on a new branch
# claude/<TaskId> from -Base (default main), then builds -Target (default
# hydra_tests) there from nothing with build_cpp.ps1.
#
# The build runs inside one of the machine-wide build slots. tools\
# build_slot.ps1 holds that rule (how many slots, the lock files, stale-owner
# takeover, the wait); this script only calls it.
#
# Why cold builds and not a copied build folder: seeding from the main
# checkout's build-cpp saves too little to be worth its risk. Two things eat
# the saving. That folder trails main (other sessions build only some targets
# there), so most objects are stale anyway. And every object that bakes in a
# path #define (HYDRA_TESTDATA_DIR and friends) has to recompile in a new tree.
# Seeding also needs surgery on MSBuild's tracking logs, whose mistakes are
# silent wrong binaries. So this helper only staggers.

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

# build_slot.ps1 owns the slot rule: it waits for a free slot, builds, and
# gives the slot back, pass or fail. The clock includes any wait.
$clock = [Diagnostics.Stopwatch]::StartNew()
& (Join-Path $PSScriptRoot "build_slot.ps1") -Repo $wt -Target $Target
$code = $LASTEXITCODE
$secs = [int]$clock.Elapsed.TotalSeconds
if ($code) {
    Write-Host "build of $Target failed in $wt after $secs s (exit $code)"
    exit 1
}
Write-Host "worktree $wt on $branch, $Target built in $secs s"
