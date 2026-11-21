#Requires -Version 7.0
<#
Self-test for the build pass: tools/build_slot.ps1, the claim in build_cpp.ps1,
and tools/new_worktree.ps1, which no longer builds.

    pwsh -NoProfile -File tools/test_build_pass.ps1

It compiles nothing and takes no real slot. Every case uses its own slot folder
in a temporary fixture. The fixture holds a copy of the real build_cpp.ps1,
build_slot.ps1, cmake_presets.ps1, stale_toolchain.ps1 and CMakePresets.json, with a find_cmake.ps1
that hands back a fake cmake. The fake writes down its arguments and what the
slot folder holds at that moment.

tools/test_hydra_ui_mcp.py covers the hydra_build_pass tool. Nothing here runs
build_slot.ps1 -Hold, which holds one slot across several builds. A real compile,
and whether MSBuild reads Directory.Build.targets at all, needs a real build,
so the main session checks those.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$slotScript = Join-Path $PSScriptRoot 'build_slot.ps1'
$repoRoot = Split-Path $PSScriptRoot

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

$fixture = Join-Path ([IO.Path]::GetTempPath()) ("build_pass_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
$caseNumber = 0

# A fresh, empty slot folder, which the rest of the case uses through the
# environment. The case runs ungated with no pass until it says otherwise.
function New-Case {
    $script:caseNumber++
    $script:slots = Join-Path $fixture "slots$script:caseNumber"
    New-Item -ItemType Directory -Path $script:slots | Out-Null
    $env:HYDRA_BUILD_SLOT_DIR = $script:slots
    Remove-Item Env:CLAUDECODE, Env:HYDRA_BUILD_PASS, Env:FAKE_CMAKE_EXIT -ErrorAction SilentlyContinue
    Set-Content $cmakeLog -Value @() -NoNewline
}

function Get-Locks { @(Get-ChildItem -LiteralPath $script:slots -ErrorAction SilentlyContinue) }

# A pwsh run of $Script: its exit code, its output lines and all of it as text.
function Invoke-Pwsh([string]$Script, [string[]]$ScriptArgs = @()) {
    $out = @(& pwsh -NoProfile -File $Script @ScriptArgs 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $LASTEXITCODE; Lines = $out; Text = ($out -join "`n") }
}

function Invoke-Slot([string[]]$SlotArgs) { Invoke-Pwsh $slotScript $SlotArgs }

function Invoke-Build([string[]]$BuildArgs = @('-Target', 'hydra_tests')) { Invoke-Pwsh $fakeRepoBuild $BuildArgs }

# Asks for a pass and returns it, or empty on a refusal.
function New-Pass([string]$Session, [int]$Holder = $PID) {
    $r = Invoke-Slot @('-NewPass', '-Session', $Session, '-Holder', "$Holder")
    if ($r.Code -eq 0) { $r.Lines[0] -replace '^pass: ', '' } else { '' }
}

function Get-CMakeLines { @(Get-Content -LiteralPath $cmakeLog) }

# What one logged cmake call was, read from its own arguments: a configure
# names a preset and does not build; the real build builds the target
# Invoke-Build asks for.
function Test-ConfigureLine([string]$Line) { $Line -match '^--preset \S+ \|' }
function Test-TargetBuildLine([string]$Line) { $Line -match '^--build .*--target hydra_tests( |$)' }

# A process that stays alive, which the build is not a child of.
function Start-Bystander { Start-Process pwsh -ArgumentList '-NoProfile', '-Command', 'Start-Sleep -Seconds 60' -PassThru -WindowStyle Hidden }

# The id of a process that has already ended.
function Get-DeadPid {
    $p = Start-Process cmd -ArgumentList '/c', 'exit' -PassThru -WindowStyle Hidden
    $p.WaitForExit()
    $p.Id
}

function Write-Lock([int]$Number, [string]$Text) {
    [IO.File]::WriteAllText((Join-Path $script:slots "slot$Number.lock"), $Text)
}

$callerEnv = @{}
foreach ($name in 'CLAUDECODE', 'HYDRA_BUILD_PASS', 'HYDRA_BUILD_SLOT_DIR') { $callerEnv[$name] = [Environment]::GetEnvironmentVariable($name) }

try {
    # ------------------------------------------------------------ the fixture
    $fakeRepo = Join-Path $fixture 'repo'
    $fakeRepoBuild = Join-Path $fakeRepo 'build_cpp.ps1'
    $cmakeLog = Join-Path $fixture 'cmake.log'
    New-Item -ItemType Directory -Force -Path (Join-Path $fakeRepo 'tools') | Out-Null
    Copy-Item (Join-Path $repoRoot 'build_cpp.ps1') $fakeRepo
    Copy-Item (Join-Path $repoRoot 'CMakePresets.json') $fakeRepo
    Copy-Item $slotScript, (Join-Path $PSScriptRoot 'cmake_presets.ps1'), (Join-Path $PSScriptRoot 'stale_toolchain.ps1') (Join-Path $fakeRepo 'tools')
    $fakeCMake = Join-Path $fixture 'fake_cmake.ps1'
    Set-Content (Join-Path $fakeRepo 'tools\find_cmake.ps1') "function Find-CMake { '$fakeCMake' }"
    Set-Content $fakeCMake @'
$locks = @(Get-ChildItem -LiteralPath $env:HYDRA_BUILD_SLOT_DIR -ErrorAction SilentlyContinue)
$holders = @($locks | ForEach-Object { (Get-Content -LiteralPath $_.FullName)[0] }) -join ','
$claimed = @($locks | Where-Object { -not (Select-String -LiteralPath $_.FullName -Pattern '^expires=' -Quiet) }).Count
Add-Content -LiteralPath $env:FAKE_CMAKE_LOG "$($args -join ' ') | locks=$($locks.Count) claimed=$claimed holders=$holders"
if ($env:FAKE_CMAKE_EXIT) { exit ([int]$env:FAKE_CMAKE_EXIT) }
exit 0
'@
    $env:FAKE_CMAKE_LOG = $cmakeLog
    # Only here so build_cpp.ps1 skips entering a Visual Studio shell for a
    # Ninja preset; nothing runs it.
    Set-Content (Join-Path $fixture 'cl.exe') ''
    $env:PATH = "$fixture;$env:PATH"

    # -------------------------------------------------------- handing out passes

    New-Case
    $a = New-Pass 'session-a'
    $b = New-Pass 'session-b'
    $c = New-Pass 'session-c'
    Check ($a -and $b -and $c -and $a -ne $b -and $b -ne $c) "three sessions: expected three different passes, got '$a' '$b' '$c'"
    Check ((Get-Locks).Count -eq 3) "three passes: expected three locks, got $((Get-Locks).Count)"
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-d')
    Check ($r.Code -eq 1 -and $r.Text -match 'all 3 build slots are taken') "fourth pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    foreach ($s in 'session-a', 'session-b', 'session-c') { Check ($r.Text -match $s) "fourth pass: the refusal does not name ${s}:`n$($r.Text)" }
    Check ((Get-Locks).Count -eq 3) 'fourth pass: a fourth lock appeared'

    New-Case
    $first = New-Pass 'session-a'
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a')
    Check ($first -and $r.Code -eq 1 -and $r.Text -match 'this session already holds one') "second pass for one session: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    Check ((New-Pass 'session-b') -ne '') 'a second session could not get a pass beside the first'

    # The limit the reply names is the one written on the lock: the lock's
    # expires= time, less the seconds the reply gives, falls between the
    # moments just before and just after the pass was asked for. The number
    # itself is build_slot.ps1's to choose.
    New-Case
    $asked = [DateTime]::UtcNow
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a')
    $answered = [DateTime]::UtcNow
    $token = $r.Lines[0] -replace '^pass: ', ''
    $named = if ($r.Text -match 'build slot (\d+) of \d+ and runs out if nothing claims it in (\d+) s') { [pscustomobject]@{ Slot = $Matches[1]; Seconds = [int]$Matches[2] } }
    Check ($r.Code -eq 0 -and $named) "the reply does not name its slot and limit:`n$($r.Text)"
    if ($named) {
        $lockLines = @(Get-Content -LiteralPath (Join-Path $script:slots "slot$($named.Slot).lock"))
        $expiresLine = @($lockLines | Where-Object { $_ -like 'expires=*' })
        Check ($expiresLine.Count -eq 1) "the pass's lock has no single expires= line:`n$($lockLines -join "`n")"
        if ($expiresLine.Count -eq 1) {
            $expires = [DateTime]::Parse(($expiresLine[0] -replace '^expires=', ''), [Globalization.CultureInfo]::InvariantCulture, [Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime()
            $writtenAt = $expires.AddSeconds(-$named.Seconds)
            Check ($writtenAt -ge $asked -and $writtenAt -le $answered) "the reply's $($named.Seconds) s is not the lock's limit: the lock expires $($expires.ToString('o')), the pass was asked for between $($asked.ToString('o')) and $($answered.ToString('o'))"
        }
    }
    Check ($token -and $r.Lines[2].Contains($token)) "the reply's command does not hold the pass '$token':`n$($r.Text)"

    New-Case
    foreach ($bad in @(), @('-Session', ''), @('-Session', 'two words')) {
        $r = Invoke-Slot (@('-NewPass') + $bad)
        Check ($r.Code -eq 1 -and $r.Text -match 'cannot be told from another') "unusable session id ($($bad -join ' ')): expected a refusal, got exit $($r.Code):`n$($r.Text)"
    }
    Check ((Get-Locks).Count -eq 0) 'an unusable session id left a lock'

    # Many askers at once get no more than the rule allows. All six start
    # together, so each one's check and write must be a single step.
    foreach ($names in @{ Want = 3; Sessions = 1..6 | ForEach-Object { "race-$_" } }, @{ Want = 1; Sessions = @('same') * 6 }) {
        New-Case
        $runs = @($names.Sessions | ForEach-Object -Begin { $n = 0 } -Process {
                $n++
                $outFile = Join-Path $script:slots "..\race$script:caseNumber-$n.txt"
                [pscustomobject]@{ Out = $outFile; Process = (Start-Process pwsh -ArgumentList '-NoProfile', '-File', $slotScript, '-NewPass', '-Session', $_ -PassThru -NoNewWindow -RedirectStandardOutput $outFile) }
            })
        foreach ($run in $runs) { $run.Process.WaitForExit() }
        $won = @($runs | Where-Object { (Get-Content -LiteralPath $_.Out -First 1) -like 'pass: *' })
        Check ($won.Count -eq $names.Want) "race: expected $($names.Want) passes from six askers, got $($won.Count)"
        Check ((Get-Locks).Count -eq $names.Want) "race: expected $($names.Want) locks, got $((Get-Locks).Count)"
    }

    # ----------------------------------------------------- what frees a slot

    # A pass past its limit frees the slot, and can no longer be claimed.
    New-Case
    $old = New-Pass 'session-a'
    New-Pass 'session-b' | Out-Null
    New-Pass 'session-c' | Out-Null
    $lock = Join-Path $script:slots 'slot1.lock'
    $text = [IO.File]::ReadAllText($lock) -replace 'expires=.*', 'expires=2020-01-01T00:00:00.0000000Z'
    [IO.File]::WriteAllText($lock, $text)
    $env:CLAUDECODE = '1'
    $env:HYDRA_BUILD_PASS = $old
    $r = Invoke-Build
    Check ($r.Code -ne 0 -and $r.Text -match 'not live' -and (Get-CMakeLines).Count -eq 0) "claiming a pass past its limit: expected a refusal before configure, got exit $($r.Code):`n$($r.Text)"
    Remove-Item Env:HYDRA_BUILD_PASS
    Check ((New-Pass 'session-d') -ne '') 'a pass past its limit did not free its slot'
    Check ((New-Pass 'session-a') -eq '') 'session-d should now hold the third slot'

    # A session whose pass ran out can ask again.
    New-Case
    New-Pass 'session-a' | Out-Null
    $lock = Join-Path $script:slots 'slot1.lock'
    [IO.File]::WriteAllText($lock, ([IO.File]::ReadAllText($lock) -replace 'expires=.*', 'expires=2020-01-01T00:00:00.0000000Z'))
    Check ((New-Pass 'session-a') -ne '') 'a session whose pass ran out could not get a new one'

    # A dead owner frees the slot, and a live one holds it.
    New-Case
    $dead = Get-DeadPid
    Write-Lock 1 "$dead`nC:\wt\one`npass=aaaa`nsession=session-x"
    New-Pass 'session-a' | Out-Null
    New-Pass 'session-b' | Out-Null
    Check ((New-Pass 'session-c') -ne '') 'a dead owner did not free its slot'
    Check ((Get-Content (Join-Path $script:slots 'slot1.lock') -Raw) -notmatch 'session-x') 'the dead owner''s lock is still there'

    New-Case
    Write-Lock 1 "$PID`nC:\wt\one`npass=aaaa`nsession=session-x"
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-x')
    Check ($r.Code -eq 1 -and $r.Text -match "process $PID, building C:\\wt\\one") "a claimed pass does not count for its session:`n$($r.Text)"

    # ------------------------------------------------- locks in the old form

    # Two lines, as every old script wrote and a plain slot still writes.
    New-Case
    Write-Lock 1 "$PID`nC:\wt\old"
    Write-Lock 2 "$(Get-DeadPid)`nC:\wt\gone"
    Write-Lock 3 ''
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a')
    Check ($r.Code -eq 0) "old-form locks: a live one, a dead one and an empty one should leave room, got exit $($r.Code):`n$($r.Text)"
    Check ((Test-Path (Join-Path $script:slots 'slot1.lock')) -and (Get-Content (Join-Path $script:slots 'slot1.lock') -Raw) -eq "$PID`nC:\wt\old") 'a live old-form lock was touched'

    New-Case
    Write-Lock 1 "$PID`nC:\wt\old"
    Write-Lock 2 "$PID`nC:\wt\old2"
    Write-Lock 3 "$PID`nC:\wt\old3"
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a')
    Check ($r.Code -eq 1 -and $r.Text -match "no session, process $PID, building C:\\wt\\old3") "old-form locks: expected a refusal naming them, got exit $($r.Code):`n$($r.Text)"

    New-Case
    Write-Lock 1 "not a number`nnonsense"
    Write-Lock 2 "$PID`nC:\wt\x`npass=bbbb`nsession=s`nexpires=not a date"
    (Get-Item (Join-Path $script:slots 'slot1.lock')).LastWriteTimeUtc = [DateTime]::UtcNow.AddMinutes(-1)
    New-Pass 'session-a' | Out-Null
    New-Pass 'session-b' | Out-Null
    Check ((New-Pass 'session-c') -ne '') 'a lock full of nonsense was not cleared'

    # --------------------------------------------------- the gate in build_cpp

    # No pass under the gate: stops before configure.
    New-Case
    $env:CLAUDECODE = '1'
    $r = Invoke-Build
    Check ($r.Code -ne 0 -and $r.Text -match 'no build pass' -and $r.Text -match 'Agents never build') "no pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    Check ((Get-CMakeLines).Count -eq 0) "no pass: cmake ran:`n$((Get-CMakeLines) -join "`n")"

    # A pass nobody issued.
    New-Case
    $env:CLAUDECODE = '1'
    $env:HYDRA_BUILD_PASS = 'made-up'
    $r = Invoke-Build
    Check ($r.Code -ne 0 -and $r.Text -match 'not live' -and (Get-CMakeLines).Count -eq 0) "made-up pass: expected a refusal before configure, got exit $($r.Code):`n$($r.Text)"

    # Off the gate: builds as it always did, with no slot and no slot folder.
    foreach ($value in $null, '0') {
        New-Case
        $env:CLAUDECODE = $value
        Remove-Item -LiteralPath $script:slots
        $r = Invoke-Build
        $said = Get-CMakeLines
        Check ($r.Code -eq 0 -and @($said | Where-Object { Test-ConfigureLine $_ }).Count -eq 1 -and @($said | Where-Object { Test-TargetBuildLine $_ }).Count -eq 1) "CLAUDECODE '$value': expected a configure and a build of hydra_tests, got exit $($r.Code):`n$($r.Text)`n$($said -join "`n")"
        Check (-not ($said -match 'lowPriority')) "CLAUDECODE '$value': the build ran at low priority:`n$($said -join "`n")"
        Check (-not (Test-Path -LiteralPath $script:slots)) "CLAUDECODE '$value': the slot folder was touched"
        New-Item -ItemType Directory -Path $script:slots | Out-Null
    }

    # With a pass: claims before configure, gives the slot back, low priority
    # on the real build line only, and the pass is spent.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $r = Invoke-Build
    $said = Get-CMakeLines
    Check ($r.Code -eq 0) "with a pass: exit $($r.Code):`n$($r.Text)"
    Check ($said.Count -eq 3 -and @($said | Where-Object { $_ -notmatch 'locks=1 claimed=1 ' }).Count -eq 0) "with a pass: every cmake step should run inside one claimed lock:`n$($said -join "`n")"
    Check ($said.Count -eq 3 -and (Test-TargetBuildLine $said[2]) -and $said[2] -match ' -lowPriority \|') "with a pass: the third step should be the build of hydra_tests at low priority:`n$($said -join "`n")"
    Check ((Test-ConfigureLine $said[0]) -and $said[0] -notmatch 'lowPriority') "with a pass: the first step should be the configure, not at low priority:`n$($said -join "`n")"
    Check ($said.Count -eq 3 -and $said[1] -match 'ZERO_CHECK' -and $said[1] -notmatch 'lowPriority') "with a pass: the ZERO_CHECK line is wrong:`n$($said -join "`n")"
    Check ((Get-Locks).Count -eq 0) 'with a pass: the slot was not given back'
    $r = Invoke-Build
    Check ($r.Code -ne 0 -and $r.Text -match 'not live') "a spent pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"

    # A Ninja preset has no MSBuild, so no low-priority switch.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $r = Invoke-Build @('-Preset', 'asan-ninja', '-Target', 'hydra_tests')
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and @($said | Where-Object { Test-TargetBuildLine $_ }).Count -eq 1 -and -not ($said -match 'lowPriority')) "Ninja preset: expected one build of hydra_tests, not at low priority; exit $($r.Code), lines:`n$($said -join "`n")`n$($r.Text)"
    Check ((Get-Locks).Count -eq 0) 'Ninja preset: the slot was not given back'

    # A failed build gives the slot back too.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $env:FAKE_CMAKE_EXIT = '3'
    $r = Invoke-Build
    Check ($r.Code -ne 0 -and (Get-Locks).Count -eq 0) "failed build: expected a failure and a free slot, got exit $($r.Code), $((Get-Locks).Count) locks"

    # One pass cannot run two builds: a live stranger's claim refuses.
    New-Case
    $bystander = Start-Bystander
    try {
        $env:HYDRA_BUILD_PASS = 'cccc'
        $env:CLAUDECODE = '1'
        Write-Lock 1 "$($bystander.Id)`nC:\wt\first`npass=cccc`nsession=session-a"
        $r = Invoke-Build
        Check ($r.Code -ne 0 -and $r.Text -match 'in use' -and (Get-CMakeLines).Count -eq 0) "second build on one pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"
        Check ((Get-Locks).Count -eq 1) 'second build on one pass: the first build''s lock was lost'
    } finally { Stop-Process -Id $bystander.Id -Force -ErrorAction SilentlyContinue }

    # ----------------------------- a long-lived shell that builds in-process

    # The shell's id goes on the lock and the shell outlives the build, so only
    # the release can free the slot. Passing and failing builds both must.
    $shell = Join-Path $fixture 'shell.ps1'
    Set-Content $shell @'
param($Build, $Slots)
try { & $Build -Target hydra_tests } catch { "threw: $_" }
"shell pid: $PID"
"locks after: $(@(Get-ChildItem -LiteralPath $Slots).Count)"
'@
    foreach ($fail in '', '3') {
        New-Case
        $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
        $env:CLAUDECODE = '1'
        $env:FAKE_CMAKE_EXIT = $fail
        $r = Invoke-Pwsh $shell @($fakeRepoBuild, $script:slots)
        $label = if ($fail) { 'failing' } else { 'passing' }
        Check ($r.Text -match '(?m)^locks after: 0$') "in-process $label build: the slot stayed taken while the shell lived:`n$($r.Text)"
        $shellPid = if ($r.Text -match '(?m)^shell pid: (\d+)$') { $Matches[1] } else { 'none' }
        Check (@(Get-CMakeLines | Where-Object { $_ -notmatch "holders=$shellPid$" }).Count -eq 0 -and (Get-CMakeLines).Count -ge 1) "in-process $label build: the lock did not hold the shell's id $shellPid`:`n$((Get-CMakeLines) -join "`n")"
    }

    # ---------------------------------------------- -Repo: exactly one slot

    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and $said.Count -eq 3) "-Repo under a pass: exit $($r.Code):`n$($r.Text)"
    Check (@($said | Where-Object { $_ -notmatch 'locks=1 claimed=1 ' }).Count -eq 0) "-Repo under a pass: expected exactly one slot taken:`n$($said -join "`n")"
    Check ((Get-Locks).Count -eq 0) '-Repo under a pass: the slot was not given back'

    New-Case
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-SlotDir', $script:slots)
    Check ($r.Code -ne 0 -and $r.Text -match 'no build pass' -and (Get-CMakeLines).Count -eq 0) "-Repo with no pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"

    # An old release's build_cpp.ps1 claims nothing, so -Repo's own claim is
    # what holds the slot through its build. Its exit code comes through.
    New-Case
    $oldRepo = Join-Path $fixture 'old_release'
    New-Item -ItemType Directory -Force $oldRepo | Out-Null
    Set-Content (Join-Path $oldRepo 'build_cpp.ps1') "& '$fakeCMake' old-build; exit 5"
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $oldRepo, '-SlotDir', $script:slots)
    Check ($r.Code -eq 5) "-Repo on an old release: expected its exit 5, got $($r.Code):`n$($r.Text)"
    Check ((Get-CMakeLines) -match 'locks=1 claimed=1 ') "-Repo on an old release: the claim did not hold through the build:`n$((Get-CMakeLines) -join "`n")"
    Check ((Get-Locks).Count -eq 0) '-Repo on an old release: the slot was not given back'

    # Off the gate, -Repo takes a plain slot and needs no pass, as it always did.
    New-Case
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and $said.Count -eq 3 -and @($said | Where-Object { $_ -notmatch 'locks=1 ' }).Count -eq 0) "-Repo off the gate: expected one plain slot:`n$($r.Text)`n$($said -join "`n")"
    Check ((Get-Locks).Count -eq 0) '-Repo off the gate: the slot was not given back'

    # ------------------------------------------------ the plain slot beside passes

    New-Case
    New-Pass 'session-a' | Out-Null
    New-Pass 'session-b' | Out-Null
    $r = Invoke-Slot @('-DryRun', '-SlotDir', $script:slots)
    Check ($r.Code -eq 0 -and $r.Text -match 'slot3\.lock taken' -and $r.Text -match 'released: True') "-DryRun beside two passes: expected slot 3, got:`n$($r.Text)"

    # All three taken: the plain route waits, as it always did.
    New-Case
    New-Pass 'session-a' | Out-Null
    New-Pass 'session-b' | Out-Null
    New-Pass 'session-c' | Out-Null
    $waitOut = Join-Path $fixture 'wait.txt'
    $waiter = Start-Process pwsh -ArgumentList '-NoProfile', '-File', $slotScript, '-DryRun', '-SlotDir', $script:slots -PassThru -NoNewWindow -RedirectStandardOutput $waitOut
    try {
        $ended = $waiter.WaitForExit(5000)
        Check (-not $ended) 'the plain route did not wait behind three passes'
        Check ((Get-Content -LiteralPath $waitOut -Raw) -match 'waiting for a build slot') 'the plain route did not say it was waiting'
    } finally { Stop-Process -Id $waiter.Id -Force -ErrorAction SilentlyContinue }

    # ---------------------------------------------------------- the second lock

    New-Case
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Check')
    Check ($r.Code -eq 1 -and $r.Text -match 'no claimed build pass') "-Check with no pass: exit $($r.Code):`n$($r.Text)"
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $r = Invoke-Slot @('-Check')
    Check ($r.Code -eq 1) 'a pass nobody claimed got past -Check, so a hand-run cmake would build holding no claim'
    $token = $env:HYDRA_BUILD_PASS
    Write-Lock 1 "$PID`nC:\wt\one`npass=$token`nsession=session-a"
    $r = Invoke-Slot @('-Check')
    Check ($r.Code -eq 0) "-Check under a live claim: exit $($r.Code):`n$($r.Text)"
    Write-Lock 1 "$(Get-DeadPid)`nC:\wt\one`npass=$token`nsession=session-a"
    $r = Invoke-Slot @('-Check')
    Check ($r.Code -eq 1) 'a dead claim got past -Check'
    Remove-Item Env:CLAUDECODE, Env:HYDRA_BUILD_PASS
    $r = Invoke-Slot @('-Check')
    Check ($r.Code -eq 0) "-Check off the gate: exit $($r.Code):`n$($r.Text)"

    # The file itself. Whether MSBuild reads it is the main session's check.
    [xml]$targets = Get-Content -LiteralPath (Join-Path $repoRoot 'Directory.Build.targets') -Raw
    $target = $targets.Project.Target
    Check ($targets.Project.InitialTargets -eq $target.Name) 'Directory.Build.targets: the check is not an initial target'
    Check ($target.Condition -eq "'`$(CLAUDECODE)' == '1'") "Directory.Build.targets: the condition is '$($target.Condition)'"
    Check ($target.Exec.Command -eq 'pwsh -NoProfile -File "$(MSBuildThisFileDirectory)tools\build_slot.ps1" -Check') "Directory.Build.targets: the command is '$($target.Exec.Command)'"

    # ---------------------------------------------------- an old script's lock being written

    # An old script creates its lock empty and fills it a moment later; the
    # empty lock counts as a taken slot, so a third live slot is not given away.
    New-Case
    $bystander = Start-Bystander
    try {
        Write-Lock 1 "$($bystander.Id)`nplain"
        Write-Lock 2 "$($bystander.Id)`nplain"
        Write-Lock 3 ''
        $r = Invoke-Slot @('-NewPass', '-Session', 'young', '-Holder', "$PID")
        Check ($r.Code -eq 1) "a young empty lock was cleared, and a pass given:`n$($r.Text)"
        Check (Test-Path (Join-Path $script:slots 'slot3.lock')) 'a young empty lock was deleted'
        (Get-Item (Join-Path $script:slots 'slot3.lock')).LastWriteTimeUtc = [DateTime]::UtcNow.AddMinutes(-1)
        $r = Invoke-Slot @('-NewPass', '-Session', 'young', '-Holder', "$PID")
        Check ($r.Code -eq 0) "an old empty lock still held a slot:`n$($r.Text)"
    } finally { Stop-Process -Id $bystander.Id -Force -ErrorAction SilentlyContinue }

    # ---------------------------------------------------- new_worktree builds nothing

    New-Case
    $gitRepo = Join-Path $fixture 'git_repo'
    New-Item -ItemType Directory -Force (Join-Path $gitRepo 'tools') | Out-Null
    Copy-Item (Join-Path $PSScriptRoot 'new_worktree.ps1') (Join-Path $gitRepo 'tools')
    $marker = Join-Path $fixture 'built.txt'
    Set-Content (Join-Path $gitRepo 'build_cpp.ps1') "Set-Content '$marker' built"
    foreach ($gitArgs in @('init', '-q', '-b', 'main'), @('add', '-A'), @('-c', 'user.name=t', '-c', 'user.email=t@example.invalid', 'commit', '-q', '-m', 'fixture')) {
        & git -C $gitRepo @gitArgs 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "git $($gitArgs -join ' ') failed in the fixture" }
    }
    $r = Invoke-Pwsh (Join-Path $gitRepo 'tools\new_worktree.ps1') @('-TaskId', 'task-1')
    Check ($r.Code -eq 0 -and (Test-Path (Join-Path $gitRepo '.claude\worktrees\task-1\build_cpp.ps1'))) "new_worktree: expected a worktree, got exit $($r.Code):`n$($r.Text)"
    Check (-not (Test-Path $marker)) 'new_worktree: it ran a build'
    & git -C $gitRepo rev-parse --verify --quiet refs/heads/claude/task-1 | Out-Null
    Check ($LASTEXITCODE -eq 0) 'new_worktree: the branch claude/task-1 is missing'
} finally {
    foreach ($name in $callerEnv.Keys) { [Environment]::SetEnvironmentVariable($name, $callerEnv[$name]) }
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_build_pass: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_build_pass: PASS, $($script:Passes) checks"
exit 0
