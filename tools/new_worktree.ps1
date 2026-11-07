# Make a task worktree. It builds nothing.
#
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p35-c2
#   pwsh -NoProfile -File tools\new_worktree.ps1 -TaskId p3-k2 -Base claude/p3-k1
#
# It makes <main checkout>\.claude\worktrees\<TaskId> on a new branch
# claude/<TaskId> from -Base (default main). The main session builds there
# later, under a build pass (tools\build_slot.ps1 owns that rule).
#
# Why the worktree starts with no build folder, not a copied one: seeding from
# the main checkout's build-cpp saves too little to be worth its risk. Two
# things eat the saving. That folder trails main (other sessions build only
# some targets there), so most objects are stale anyway. And every object that
# bakes in a path #define (HYDRA_TESTDATA_DIR and friends) has to recompile in
# a new tree. Seeding also needs surgery on MSBuild's tracking logs, whose
# mistakes are silent wrong binaries.

param(
    [Parameter(Mandatory = $true)][ValidatePattern('^[A-Za-z0-9._-]+$')][string]$TaskId,
    [string]$Base = "main"
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

Write-Host "worktree $wt on $branch"
