# The machine-wide build slots and the build pass.
#
# This file is the one place that knows the slot rule, the pass rule and their
# numbers. Think of a ticket booth. A slot is a seat; a pass is a ticket for
# one seat. Under Claude Code a build needs a pass, and only a main session can
# get one (a hook refuses the tool to agents). Everywhere else, such as CI and
# a person's terminal, nothing here gates anything: Test-BuildPassGated says
# which side a run is on.
#
# Each slot is a lock file, C:\Users\Patrick\.claude\hooks\state\build_slots\
# slot1.lock and up. Read-BuildSlot says what a lock holds and
# Test-BuildSlotLive says when one still counts. A lock that no longer counts
# is stale, so the next caller clears it.
#
# Ask for a pass (the hydra_build_pass tool does this; it never waits):
#     pwsh -NoProfile -File tools\build_slot.ps1 -NewPass -Session <session id> -Preset <preset> [-Holder <process id>]
# It prints the pass and exits 0, or names who holds the slots and exits 1.
# -Preset is the build preset the pass is for, and it has no default. The
# pass builds that preset and no other: Enter-BuildPass owns that check.
# -Holder is the process that asks. It goes on the lock's first line, so a
# script on an old branch, which reads only that line, sees a busy slot.
#
# Use a pass. The pass travels in the environment, in HYDRA_BUILD_PASS:
#     & tools\build_slot.ps1 -Claim -Owner <what is building> -Preset <preset>
#     & tools\build_slot.ps1 -Hold -Owner <what is building> -Preset <preset>
#     & tools\build_slot.ps1 -Release <the Lock field of what they returned>
# Call them with & in the process that builds, because a claim belongs to that
# process. Each returns an object: Gated says whether the run is under the
# gate, and Lock is the lock this caller must release, in a finally. A long-
# lived shell that builds in-process keeps its process id after the build, so
# only that finally frees the slot. Lock is empty when the caller has nothing
# to release. -Claim is what build_cpp.ps1 uses. -Hold is for a script that
# builds several times or around another script: outside the gate it waits for
# a plain slot, as these scripts always have. A build started inside a claim
# (by the same process or a child of it) rides on that claim and takes no
# second slot; Enter-BuildPass owns that rule. A pass serves one claim: after
# the release it is spent.
#
# Check that a build runs under a claimed pass (Directory.Build.targets does):
#     pwsh -NoProfile -File tools\build_slot.ps1 -Check
#
# Run a build in another worktree, under the pass the caller holds:
#     pwsh -NoProfile -File tools\build_slot.ps1 -Repo <worktree> [-Preset <preset>] [-Target hydra_tests]
# It claims with -Preset and hands it to build_cpp.ps1, so under a pass for a
# preset it must name that preset. Without -Preset, build_cpp.ps1 gets none
# and uses its own default.
# It runs <worktree>\build_cpp.ps1 and exits with the build's result: 0 when
# it passed, otherwise its exit code, or 1 when it threw. It holds the claim
# itself, so an old release whose build_cpp.ps1 claims nothing still sits in
# its slot. A very long -Repo path can break MSBuild's 260-character limit, so
# put old-release worktrees on a short path.
#
# -SlotDir points at another folder, so a test never touches a real slot.
# HYDRA_BUILD_SLOT_DIR does the same for a script that calls this one:
#     pwsh -NoProfile -File tools\build_slot.ps1 -DryRun -SlotDir <scratch folder>
# takes one slot there, prints the lock, releases it and exits.
#
# Call this script; do not dot-source it, because its parameters would replace
# the caller's.

param(
    [string]$Repo = '',
    [string]$Target = '',
    [string]$SlotDir = '',
    [switch]$DryRun,
    [switch]$NewPass,
    [string]$Session = '',
    [string]$Preset = '',
    [int]$Holder = 0,
    [switch]$Claim,
    [switch]$Hold,
    [string]$Owner = '',
    [string]$Release = '',
    [switch]$Check
)

# How many builds may run at once, how often a waiting build checks again,
# and how often it says it is still waiting. Decision D49 (2026-10-04,
# docs/audit/2026-10-03-fix-decisions.md); the user's limit of three is in
# docs/agents/rulings/agents-and-workflow.md.
$BuildSlotCount = 3
$BuildSlotPollSeconds = 15
$BuildSlotMessageSeconds = 60
$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'
# How long a pass may sit unclaimed. The user's answer in the same rulings page.
$BuildPassUnclaimedSeconds = 120
# How long a caller waits for its turn to read and write the slot folder. Each
# turn lasts milliseconds, so running out means something is stuck; the caller
# stops and says so.
$BuildSlotFolderWaitMs = 5000
# How long an empty lock file is taken to be a slot still being written.
$BuildSlotYoungSeconds = 5
$BuildPassHowTo = 'Agents never build: hand back, and the main session builds. A main session gets a pass from hydra_build_pass.'

# True when this run is under the gate: the same test the git ref gate uses.
function Test-BuildPassGated { $env:CLAUDECODE -eq '1' }

function Get-BuildSlotDir([string]$Dir) {
    if ($Dir) { return $Dir }
    if ($env:HYDRA_BUILD_SLOT_DIR) { return $env:HYDRA_BUILD_SLOT_DIR }
    $BuildSlotDefaultDir
}

# Runs $Work while no other caller reads or writes the same slot folder, so a
# check and the write it leads to happen as one step.
function Use-BuildSlotFolder([string]$Dir, [scriptblock]$Work) {
    New-Item -ItemType Directory -Force $Dir | Out-Null
    $name = 'Local\hydra_build_slots_' + ($Dir.ToLowerInvariant() -replace '[^a-z0-9]', '_')
    $mutex = [System.Threading.Mutex]::new($false, $name)
    $mine = $false
    try {
        try { $mine = $mutex.WaitOne($BuildSlotFolderWaitMs) }
        catch [System.Threading.AbandonedMutexException] { $mine = $true }
        if (-not $mine) { throw "build_slot: the slot folder $Dir stayed locked for $BuildSlotFolderWaitMs ms; something is stuck" }
        & $Work
    } finally {
        if ($mine) { $mutex.ReleaseMutex() }
        $mutex.Dispose()
    }
}

# What one lock file says. Line 1 is a process id and line 2 says what is
# building. Those two lines are the whole of a plain slot, and of every lock an
# old script wrote. A pass adds pass=, session= and preset= lines, plus
# expires= until it is claimed. A pass handed out before passes named a
# preset has no preset= line, and HasPreset says which kind a lock is. A lock
# that cannot be read comes back with no holder.
function Read-BuildSlot([string]$Path) {
    $lines = @(Get-Content -LiteralPath $Path -ErrorAction SilentlyContinue)
    $fields = @{}
    foreach ($line in ($lines | Select-Object -Skip 2)) {
        if ($line -match '^(pass|session|preset|expires)=(.*)$') { $fields[$Matches[1]] = $Matches[2] }
    }
    $expires = $null
    if ($fields.ContainsKey('expires')) {
        try { $expires = [DateTime]::Parse($fields.expires, $null, 'RoundtripKind') } catch { }
    }
    [pscustomobject]@{
        Path = $Path
        Holder = $(if ($lines.Count) { "$($lines[0])" -as [int] } else { 0 })
        What = $(if ($lines.Count -gt 1) { "$($lines[1])" } else { '' })
        Pass = $fields.pass
        Session = $fields.session
        HasPreset = $fields.ContainsKey('preset')
        Preset = $fields.preset
        Unclaimed = $fields.ContainsKey('expires')
        Expires = $expires
    }
}

# An unclaimed pass counts until it runs out. Anything else counts while its
# holder process lives.
function Test-BuildSlotLive($Lock) {
    if ($Lock.Unclaimed) { return [bool]($Lock.Expires -and [DateTime]::UtcNow -lt $Lock.Expires) }
    [bool]($Lock.Holder -and (Get-Process -Id $Lock.Holder -ErrorAction SilentlyContinue))
}

function Get-BuildSlotPath([string]$Dir, [int]$Number) { Join-Path $Dir "slot$Number.lock" }

# The locks that count, clearing the rest. Run it inside Use-BuildSlotFolder.
function Get-LiveBuildSlots([string]$Dir) {
    foreach ($i in 1..$BuildSlotCount) {
        $f = Get-BuildSlotPath $Dir $i
        if (-not (Test-Path -LiteralPath $f)) { continue }
        $lock = Read-BuildSlot $f
        # An old script makes its lock empty and then fills it, so an empty
        # lock this young is a slot being taken, not a stale one.
        $young = -not $lock.Holder -and ([DateTime]::UtcNow - (Get-Item -LiteralPath $f -ErrorAction SilentlyContinue).LastWriteTimeUtc).TotalSeconds -lt $BuildSlotYoungSeconds
        if ((Test-BuildSlotLive $lock) -or $young) { $lock } else { Remove-Item -LiteralPath $f -Force -ErrorAction SilentlyContinue }
    }
}

# Who holds a lock, in words for a refusal.
function Format-BuildSlotHolder($Lock) {
    $who = if ($Lock.Session) { "session $($Lock.Session)" } else { 'no session' }
    if ($Lock.Unclaimed) {
        $left = [int][Math]::Ceiling(($Lock.Expires - [DateTime]::UtcNow).TotalSeconds)
        return "$who, a pass not yet claimed, running out in $left s"
    }
    "$who, process $($Lock.Holder), building $($Lock.What)"
}

# Writes a lock whole, through a temporary file, so nobody reads half of one.
# With -Replace it overwrites the lock; otherwise it fails if the lock exists.
function Write-BuildSlot([string]$Path, [string[]]$Lines, [switch]$Replace) {
    $tmp = "$Path.$([guid]::NewGuid().ToString('N')).tmp"
    [System.IO.File]::WriteAllText($tmp, ($Lines -join "`n"))
    try {
        if ($Replace) { [System.IO.File]::Replace($tmp, $Path, [NullString]::Value) } else { [System.IO.File]::Move($tmp, $Path) }
    } finally {
        Remove-Item -LiteralPath $tmp -Force -ErrorAction SilentlyContinue
    }
}

# Hands out a pass, or says why not. Never waits for a slot. The result has
# Pass (empty when refused) and Text, the lines for the caller to show.
function New-BuildPass([string]$ForSession, [string]$ForPreset, [int]$HolderId, [string]$Dir) {
    if ($ForSession -notmatch '^\S+$') {
        return [pscustomobject]@{ Pass = ''; Text = @("no pass: the session id '$ForSession' is not usable, so this session cannot be told from another") }
    }
    # One word, because it goes on one lock line and into the command below.
    if ($ForPreset -notmatch '^\S+$') {
        return [pscustomobject]@{ Pass = ''; Text = @("no pass: name the build preset this pass is for (the preset '$ForPreset' is not usable). A pass builds the preset it names and no other; there is no default.") }
    }
    Use-BuildSlotFolder $Dir {
        $live = @(Get-LiveBuildSlots $Dir)
        $own = @($live | Where-Object { $_.Session -eq $ForSession })
        if ($own.Count) {
            return [pscustomobject]@{ Pass = ''; Text = @("no pass: this session already holds one ($(Format-BuildSlotHolder $own[0])). One build at a time per session.") }
        }
        if ($live.Count -ge $BuildSlotCount) {
            $holders = @($live | ForEach-Object { Format-BuildSlotHolder $_ })
            return [pscustomobject]@{ Pass = ''; Text = @("no pass: all $BuildSlotCount build slots are taken, by:") + @($holders | ForEach-Object { "  $_" }) }
        }
        $free = 1..$BuildSlotCount | Where-Object { -not (Test-Path -LiteralPath (Get-BuildSlotPath $Dir $_)) } | Select-Object -First 1
        $token = [guid]::NewGuid().ToString('N')
        $expires = [DateTime]::UtcNow.AddSeconds($BuildPassUnclaimedSeconds).ToString('o')
        Write-BuildSlot (Get-BuildSlotPath $Dir $free) @("$HolderId", "pass for session $ForSession", "pass=$token", "session=$ForSession", "preset=$ForPreset", "expires=$expires")
        [pscustomobject]@{ Pass = $token; Text = @(
                "pass: $token",
                "It holds build slot $free of $BuildSlotCount and runs out if nothing claims it in $BuildPassUnclaimedSeconds s. It builds the preset $ForPreset only. Build with:",
                "`$env:HYDRA_BUILD_PASS='$token'; .\build_cpp.ps1 -Preset $ForPreset -Target hydra_tests") }
    }
}

# True when the process is this one or one of its parents. The walk ends at a
# process with no parent, or at one seen already, because Windows reuses ids.
function Test-BuildPassAncestor([int]$ProcessId) {
    $seen = [System.Collections.Generic.HashSet[int]]::new()
    $at = $PID
    while ($at -and $seen.Add($at)) {
        if ($at -eq $ProcessId) { return $true }
        $at = (Get-Process -Id $at -ErrorAction SilentlyContinue).Parent.Id
    }
    $false
}

# Claims the pass in HYDRA_BUILD_PASS for this process, or throws saying why
# not. Returns the lock to release, or nothing when an enclosing claim by this
# process or a parent already covers the build, because that claim's owner
# releases it. Any other live claim on the same pass refuses, which keeps one
# pass from running two builds side by side.
# A pass that names a preset builds that preset only. The check runs before the
# enclosing-claim return, so a nested build is held to it too. The two words
# must match letter for letter, capitals included, because PowerShell's
# parameter checks ignore case and the build record keeps the word as typed.
# A lock with no preset= line is claimed with any preset, or none.
function Enter-BuildPass([string]$What, [string]$Dir, [string]$ForPreset) {
    $token = $env:HYDRA_BUILD_PASS
    if (-not $token) { throw "no build pass: HYDRA_BUILD_PASS is empty. $BuildPassHowTo" }
    Use-BuildSlotFolder $Dir {
        $lock = @(Get-LiveBuildSlots $Dir | Where-Object { $_.Pass -eq $token }) | Select-Object -First 1
        if (-not $lock) { throw "this build pass is not live: it ran out, was already used, or was never issued. $BuildPassHowTo" }
        if ($lock.HasPreset -and (-not $ForPreset -or $ForPreset -cne $lock.Preset)) {
            $asked = if ($ForPreset) { "the preset '$ForPreset'" } else { 'no preset' }
            throw "this build pass is for the preset '$($lock.Preset)' only, and this build named $asked. Build with -Preset $($lock.Preset), or ask hydra_build_pass for a pass for the preset you want."
        }
        if (-not $lock.Unclaimed) {
            if (Test-BuildPassAncestor $lock.Holder) { return }
            throw "this build pass is in use: $(Format-BuildSlotHolder $lock)"
        }
        $claimedLines = @("$PID", $What, "pass=$($lock.Pass)", "session=$($lock.Session)")
        if ($lock.HasPreset) { $claimedLines += "preset=$($lock.Preset)" }
        Write-BuildSlot $lock.Path $claimedLines -Replace
        Write-Host "build pass claimed in $($lock.Path)"
        $lock.Path
    }
}

# True when HYDRA_BUILD_PASS is claimed by a live process. Reads only, so it
# clears nothing and needs no turn at the slot folder.
function Test-BuildPassClaimed([string]$Dir) {
    $token = $env:HYDRA_BUILD_PASS
    if (-not $token) { return $false }
    foreach ($i in 1..$BuildSlotCount) {
        $f = Get-BuildSlotPath $Dir $i
        if (-not (Test-Path -LiteralPath $f)) { continue }
        $lock = Read-BuildSlot $f
        if ($lock.Pass -eq $token -and -not $lock.Unclaimed -and (Test-BuildSlotLive $lock)) { return $true }
    }
    $false
}

# Takes a free plain slot, waiting until one frees. Returns the lock file's
# path. This is the route outside the gate; a pass never waits.
function Get-BuildSlot([string]$What, [string]$Dir) {
    $waited = 0
    while ($true) {
        $taken = Use-BuildSlotFolder $Dir {
            $null = @(Get-LiveBuildSlots $Dir)
            foreach ($i in 1..$BuildSlotCount) {
                $f = Get-BuildSlotPath $Dir $i
                if (Test-Path -LiteralPath $f) { continue }
                Write-BuildSlot $f @("$PID", $What)
                return $f
            }
        }
        if ($taken) {
            Write-Host "build slot $taken taken"
            return $taken
        }
        if ($waited % $BuildSlotMessageSeconds -eq 0) { Write-Host "waiting for a build slot ($waited s)" }
        Start-Sleep -Seconds $BuildSlotPollSeconds
        $waited += $BuildSlotPollSeconds
    }
}

# Gives a slot back. A reader holding the file open for a moment can make the
# delete fail, and a lock left behind would hold its slot until its owner
# process died, so it tries again.
function Release-BuildSlot([string]$Slot) {
    if (-not $Slot) { return }
    foreach ($try in 1..10) {
        Remove-Item -LiteralPath $Slot -Force -ErrorAction SilentlyContinue
        if (-not (Test-Path -LiteralPath $Slot)) { return }
        Start-Sleep -Milliseconds 200
    }
}

# The two ways in for a build. -Claim is for build_cpp.ps1, which outside the
# gate takes nothing. With -Wait, outside the gate it waits for a plain slot.
function Enter-BuildSlot([string]$What, [string]$Dir, [string]$ForPreset, [switch]$Wait) {
    if (Test-BuildPassGated) {
        return [pscustomobject]@{ Gated = $true; Lock = "$(Enter-BuildPass $What $Dir $ForPreset)" }
    }
    $lock = if ($Wait) { Get-BuildSlot $What $Dir } else { '' }
    [pscustomobject]@{ Gated = $false; Lock = $lock }
}

$dir = Get-BuildSlotDir $SlotDir

if ($NewPass) {
    $result = New-BuildPass $Session $Preset $Holder $dir
    $result.Text | ForEach-Object { Write-Output $_ }
    exit $(if ($result.Pass) { 0 } else { 1 })
}
if ($Claim) {
    Enter-BuildSlot $Owner $dir $Preset
    exit 0
}
if ($Hold) {
    Enter-BuildSlot $Owner $dir $Preset -Wait
    exit 0
}
if ($PSBoundParameters.ContainsKey('Release')) {
    Release-BuildSlot $Release
    exit 0
}
if ($Check) {
    if (-not (Test-BuildPassGated) -or (Test-BuildPassClaimed $dir)) { exit 0 }
    Write-Output "no claimed build pass for this build. $BuildPassHowTo"
    exit 1
}
if ($DryRun) {
    if (-not $SlotDir) { throw '-DryRun needs -SlotDir, so it never takes a real slot' }
    $slot = Get-BuildSlot 'dry run' $dir
    Write-Host "lock holds: $((Get-Content $slot) -join ' | ')"
    Release-BuildSlot $slot
    Write-Host "released: $(-not (Test-Path $slot))"
    exit 0
}
if (-not $Repo) { throw 'give -Repo <worktree>, or one of the modes this file describes at its top' }

# The build's own claim must read the same folder as this one.
if ($SlotDir) { $env:HYDRA_BUILD_SLOT_DIR = $SlotDir }
$slot = Enter-BuildSlot $Repo $dir $Preset -Wait
$code = 0
# Older checkouts' build_cpp.ps1 builds the current folder, so the build runs
# from inside -Repo; otherwise an old release would rebuild the caller's tree.
$pushed = $false
$buildArgs = @{}
if ($Preset) { $buildArgs.Preset = $Preset }
if ($Target) { $buildArgs.Target = $Target }
try {
    Push-Location -LiteralPath $Repo -ErrorAction Stop
    $pushed = $true
    & (Join-Path $Repo 'build_cpp.ps1') @buildArgs
    if ($LASTEXITCODE) { $code = $LASTEXITCODE }
} catch {
    Write-Host "build failed in ${Repo}: $_"
    $code = 1
} finally {
    if ($pushed) { Pop-Location }
    Release-BuildSlot $slot.Lock
}
exit $code
