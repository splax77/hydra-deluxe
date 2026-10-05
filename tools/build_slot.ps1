# The machine-wide build slots: at most three cold builds at once.
#
# This file is the one place that knows the slot rule. A build takes one of
# the slots before it starts and gives it back when it ends, pass or fail.
# Each slot is a lock file, C:\Users\Patrick\.claude\hooks\state\build_slots\
# slot1.lock to slot3.lock. A lock holds two lines: the owner's process id,
# then what it is building (a worktree path). A lock whose owner process is
# gone is stale, so the next build takes it over. While every slot is busy, a
# build waits in the foreground and says so now and then.
#
# Run a build in an existing worktree, inside a slot (what phase 3's
# cold_build.ps1 did):
#     pwsh -NoProfile -File tools\build_slot.ps1 -Repo <worktree> [-Target hydra_tests]
# It runs <worktree>\build_cpp.ps1 and exits with the build's result: 0 when
# it passed, otherwise its exit code, or 1 when it threw. tools\new_worktree.ps1
# calls it the same way (with &) after it makes a new worktree. Call it; do
# not dot-source it, because its -Repo and -Target would replace the caller's.
#
# -SlotDir points at another folder, for a dry run that must not take a real
# slot:
#     pwsh -NoProfile -File tools\build_slot.ps1 -DryRun -SlotDir <scratch folder>
# takes one slot there, prints the lock, releases it and exits.

param(
    [string]$Repo = '',
    [string]$Target = '',
    [string]$SlotDir = '',
    [switch]$DryRun
)

# How many builds may run at once, how often a waiting build checks again,
# and how often it says it is still waiting. Decision D49 (2026-10-04,
# docs/audit/2026-10-03-fix-decisions.md).
$BuildSlotCount = 3
$BuildSlotPollSeconds = 15
$BuildSlotMessageSeconds = 60
$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'

# Takes a free slot, waiting until one frees. Returns the lock file's path.
function Get-BuildSlot([string]$Owner, [string]$Dir = $BuildSlotDefaultDir) {
    New-Item -ItemType Directory -Force $Dir | Out-Null
    $waited = 0
    while ($true) {
        foreach ($i in 1..$BuildSlotCount) {
            $f = Join-Path $Dir "slot$i.lock"
            if (Test-Path $f) {
                # Busy while its owner process lives; stale otherwise.
                $holder = (Get-Content $f -ErrorAction SilentlyContinue | Select-Object -First 1) -as [int]
                if ($holder -and (Get-Process -Id $holder -ErrorAction SilentlyContinue)) { continue }
                Remove-Item $f -Force -ErrorAction SilentlyContinue
            }
            try {
                # CreateNew fails if another build made the file first.
                $fs = [System.IO.File]::Open($f, 'CreateNew', 'Write')
                $b = [System.Text.Encoding]::ASCII.GetBytes("$PID`n$Owner")
                $fs.Write($b, 0, $b.Length); $fs.Close()
                Write-Host "build slot $f taken"
                return $f
            } catch { }
        }
        if ($waited % $BuildSlotMessageSeconds -eq 0) { Write-Host "waiting for a build slot ($waited s)" }
        Start-Sleep -Seconds $BuildSlotPollSeconds
        $waited += $BuildSlotPollSeconds
    }
}

# Gives a slot back.
function Release-BuildSlot([string]$Slot) {
    if ($Slot) { Remove-Item $Slot -Force -ErrorAction SilentlyContinue }
}

$dir = if ($SlotDir) { $SlotDir } else { $BuildSlotDefaultDir }
if ($DryRun) {
    if (-not $SlotDir) { throw '-DryRun needs -SlotDir, so it never takes a real slot' }
    $slot = Get-BuildSlot -Owner 'dry run' -Dir $dir
    Write-Host "lock holds: $((Get-Content $slot) -join ' | ')"
    Release-BuildSlot $slot
    Write-Host "released: $(-not (Test-Path $slot))"
    exit 0
}
if (-not $Repo) { throw 'give -Repo <worktree> (or -DryRun -SlotDir <folder>)' }

$slot = Get-BuildSlot -Owner $Repo -Dir $dir
$code = 0
try {
    if ($Target) { & (Join-Path $Repo 'build_cpp.ps1') -Target $Target } else { & (Join-Path $Repo 'build_cpp.ps1') }
    if ($LASTEXITCODE) { $code = $LASTEXITCODE }
} catch {
    Write-Host "build failed in ${Repo}: $_"
    $code = 1
} finally {
    Release-BuildSlot $slot
}
exit $code
