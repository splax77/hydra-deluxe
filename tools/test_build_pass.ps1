#Requires -Version 7.0
<#
Self-test for the build pass: tools/build_slot.ps1, the claim in build_cpp.ps1,
and tools/new_worktree.ps1, which no longer builds.

    pwsh -NoProfile -File tools/test_build_pass.ps1

It compiles nothing and takes no real slot. Every case uses its own slot folder
in a temporary fixture. The fixture holds a copy of the real build_cpp.ps1,
build_slot.ps1, cmake_presets.ps1, stale_toolchain.ps1 and CMakePresets.json, with a find_cmake.ps1
that hands back a fake cmake. The fake writes down its arguments and what the
slot folder holds at that moment. The fixture repo is a git repo, so the
-Repo mode has a commit and changed paths to put in its build record.

Locks are read through build_slot.ps1's own Read-BuildSlot.

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

# Asks for a pass and returns it, or empty on a refusal. Invoke-Build builds
# the default preset unless it is told another, so that is what a case asks
# for unless it says otherwise.
function New-Pass([string]$Session, [int]$Holder = $PID, [string]$Preset = 'default', [string]$Repo = '', [string]$Target = '') {
    $passArgs = @('-NewPass', '-Session', $Session, '-Preset', $Preset, '-Holder', "$Holder")
    if ($Repo) { $passArgs += @('-Repo', $Repo) }
    if ($Target) { $passArgs += @('-Target', $Target) }
    $r = Invoke-Slot $passArgs
    if ($r.Code -eq 0) { $r.Lines[0] -replace '^pass: ', '' } else { '' }
}

# The lock reader is build_slot.ps1's own Read-BuildSlot, with the target
# splitter it calls. The script cannot be dot-sourced, because its main part
# would run, so this takes those functions out of its parsed text.
$slotAst = [System.Management.Automation.Language.Parser]::ParseFile($slotScript, [ref]$null, [ref]$null)
foreach ($name in 'Read-BuildSlot', 'Split-BuildTargets') {
    $fnAst = $slotAst.Find({ param($node) $node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -eq $name }.GetNewClosure(), $true)
    if (-not $fnAst) { throw "build_slot.ps1 has no $name function to read locks with" }
    . ([scriptblock]::Create($fnAst.Extent.Text))
}

# The build record lines in a git repo's common dir, each read as JSON.
function Get-Records([string]$Repo) {
    $file = Join-Path $Repo '.git\build_results.jsonl'
    if (-not (Test-Path -LiteralPath $file)) { return @() }
    @(Get-Content -LiteralPath $file | Where-Object { $_ } | ForEach-Object { $_ | ConvertFrom-Json })
}

function Invoke-Git([string]$Dir, [string[]]$GitArgs) {
    $out = @(& git -C $Dir -c user.name=t -c user.email=t@example.invalid @GitArgs 2>&1 | ForEach-Object { "$_" })
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' ') failed in ${Dir}:`n$($out -join "`n")" }
    $out
}

# The paths `git status --porcelain` lists, read from its plain form. The
# fixture's file names hold no spaces or quotes, so the plain form is exact.
function Get-PorcelainPaths([string]$Dir) { @(Invoke-Git $Dir @('status', '--porcelain') | Where-Object { $_ } | ForEach-Object { $_.Substring(3) }) }

function Test-SameSet($A, $B) { (@($A | Sort-Object) -join "`n") -ceq (@($B | Sort-Object) -join "`n") }

# The one lock in the case's slot folder, read by Read-BuildSlot.
function Read-OnlyLock {
    $locks = @(Get-Locks)
    if ($locks.Count -ne 1) { throw "expected one lock in $script:slots, found $($locks.Count)" }
    Read-BuildSlot $locks[0].FullName
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
    # A git repo, so the -Repo mode has a commit and changed paths to record.
    Set-Content (Join-Path $fakeRepo 'notes.txt') 'first'
    Invoke-Git $fakeRepo @('init', '-q', '-b', 'main') | Out-Null
    Invoke-Git $fakeRepo @('add', '-A') | Out-Null
    Invoke-Git $fakeRepo @('commit', '-q', '-m', 'fixture') | Out-Null

    # -------------------------------------------------------- handing out passes

    New-Case
    $a = New-Pass 'session-a'
    $b = New-Pass 'session-b'
    $c = New-Pass 'session-c'
    Check ($a -and $b -and $c -and $a -ne $b -and $b -ne $c) "three sessions: expected three different passes, got '$a' '$b' '$c'"
    Check ((Get-Locks).Count -eq 3) "three passes: expected three locks, got $((Get-Locks).Count)"
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-d', '-Preset', 'default')
    Check ($r.Code -eq 1 -and $r.Text -match 'all 3 build slots are taken') "fourth pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    foreach ($s in 'session-a', 'session-b', 'session-c') { Check ($r.Text -match $s) "fourth pass: the refusal does not name ${s}:`n$($r.Text)" }
    Check ((Get-Locks).Count -eq 3) 'fourth pass: a fourth lock appeared'

    New-Case
    $first = New-Pass 'session-a'
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default')
    Check ($first -and $r.Code -eq 1 -and $r.Text -match 'this session already holds one') "second pass for one session: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    Check ((New-Pass 'session-b') -ne '') 'a second session could not get a pass beside the first'

    # The limit the reply names is the one written on the lock: the lock's
    # expires= time, less the seconds the reply gives, falls between the
    # moments just before and just after the pass was asked for. The number
    # itself is build_slot.ps1's to choose.
    New-Case
    $asked = [DateTime]::UtcNow
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default')
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
        $r = Invoke-Slot (@('-NewPass', '-Preset', 'default') + $bad)
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
                [pscustomobject]@{ Out = $outFile; Process = (Start-Process pwsh -ArgumentList '-NoProfile', '-File', $slotScript, '-NewPass', '-Session', $_, '-Preset', 'default' -PassThru -NoNewWindow -RedirectStandardOutput $outFile) }
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
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-x', '-Preset', 'default')
    Check ($r.Code -eq 1 -and $r.Text -match "process $PID, building C:\\wt\\one") "a claimed pass does not count for its session:`n$($r.Text)"

    # ------------------------------------------------- locks in the old form

    # Two lines, as every old script wrote and a plain slot still writes.
    New-Case
    Write-Lock 1 "$PID`nC:\wt\old"
    Write-Lock 2 "$(Get-DeadPid)`nC:\wt\gone"
    Write-Lock 3 ''
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default')
    Check ($r.Code -eq 0) "old-form locks: a live one, a dead one and an empty one should leave room, got exit $($r.Code):`n$($r.Text)"
    Check ((Test-Path (Join-Path $script:slots 'slot1.lock')) -and (Get-Content (Join-Path $script:slots 'slot1.lock') -Raw) -eq "$PID`nC:\wt\old") 'a live old-form lock was touched'

    New-Case
    Write-Lock 1 "$PID`nC:\wt\old"
    Write-Lock 2 "$PID`nC:\wt\old2"
    Write-Lock 3 "$PID`nC:\wt\old3"
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default')
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
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset 'coverage-ninja'
    $env:CLAUDECODE = '1'
    $r = Invoke-Build @('-Preset', 'coverage-ninja', '-Target', 'hydra_tests')
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

    # ------------------------------------------------ a pass is for one preset

    # A pass with no preset is refused, and takes no slot.
    New-Case
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a')
    Check ($r.Code -eq 1 -and $r.Text -match 'name the build preset' -and (Get-Locks).Count -eq 0) "-NewPass with no -Preset: expected a refusal and no lock, got exit $($r.Code), $((Get-Locks).Count) locks:`n$($r.Text)"

    # A pass for mutation, claimed by a mutation build, is claimed and built.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset 'mutation'
    $env:CLAUDECODE = '1'
    $lock = Read-OnlyLock
    Check ($lock.HasPreset -and $lock.Preset -ceq 'mutation') "a pass for mutation: the lock does not hold preset=mutation (HasPreset $($lock.HasPreset), '$($lock.Preset)')"
    $r = Invoke-Build @('-Preset', 'mutation', '-Target', 'hydra_tests')
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and @($said | Where-Object { Test-TargetBuildLine $_ }).Count -eq 1 -and @($said | Where-Object { $_ -notmatch 'locks=1 claimed=1 ' }).Count -eq 0) "mutation pass, mutation build: expected one claimed build of hydra_tests, got exit $($r.Code):`n$($r.Text)`n$($said -join "`n")"

    # The same pass, claimed by a build of another preset, or of the same word
    # with a capital, is refused before configure and stays unclaimed.
    foreach ($other in 'default', 'Mutation') {
        New-Case
        $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset 'mutation'
        $env:CLAUDECODE = '1'
        $r = Invoke-Build @('-Preset', $other, '-Target', 'hydra_tests')
        Check ($r.Code -ne 0 -and $r.Text -cmatch "for the preset 'mutation' only" -and (Get-CMakeLines).Count -eq 0) "mutation pass, $other build: expected a refusal before configure, got exit $($r.Code):`n$($r.Text)`n$((Get-CMakeLines) -join "`n")"
        $lock = Read-OnlyLock
        Check ($lock.Unclaimed -and $lock.Preset -ceq 'mutation') "mutation pass, $other build: the lock was claimed or lost its preset (Unclaimed $($lock.Unclaimed), '$($lock.Preset)')"
    }

    # A claim that names no preset is refused under a pass for one. build_cpp.ps1
    # always names one, so this goes to the script itself.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset 'mutation'
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Claim', '-Owner', 'no preset')
    Check ($r.Code -ne 0 -and $r.Text -cmatch "for the preset 'mutation' only, and this build named no preset") "mutation pass, claim with no preset: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    Check ((Read-OnlyLock).Unclaimed) 'mutation pass, claim with no preset: the lock was claimed'

    # After a claim the lock keeps its preset line. The claiming process has
    # ended by the time the test reads it, which leaves the file as it was.
    New-Case
    $token = New-Pass 'session-a' -Preset 'mutation'
    $env:HYDRA_BUILD_PASS = $token
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Claim', '-Owner', 'claimer', '-Preset', 'mutation')
    $lock = Read-OnlyLock
    Check ($r.Code -eq 0 -and -not $lock.Unclaimed -and $lock.Pass -eq $token -and $lock.What -eq 'claimer') "mutation pass, mutation claim: expected a claimed lock, got exit $($r.Code), Unclaimed $($lock.Unclaimed), What '$($lock.What)':`n$($r.Text)"
    Check ($lock.HasPreset -and $lock.Preset -ceq 'mutation') "after a claim the lock lost its preset line (HasPreset $($lock.HasPreset), '$($lock.Preset)')"

    # A nested claim, under a claimed mutation pass held by this test's own
    # process, is held to the same preset. The same preset rides on the claim.
    New-Case
    $env:HYDRA_BUILD_PASS = 'dddd'
    $env:CLAUDECODE = '1'
    Write-Lock 1 "$PID`nC:\wt\outer`npass=dddd`nsession=session-a`npreset=mutation"
    $r = Invoke-Slot @('-Claim', '-Owner', 'inner', '-Preset', 'default')
    Check ($r.Code -ne 0 -and $r.Text -cmatch "for the preset 'mutation' only") "nested default claim under a mutation claim: expected a refusal, got exit $($r.Code):`n$($r.Text)"
    $r = Invoke-Slot @('-Claim', '-Owner', 'inner', '-Preset', 'mutation')
    Check ($r.Code -eq 0) "nested mutation claim under a mutation claim: expected it to ride on the claim, got exit $($r.Code):`n$($r.Text)"
    Check ((Get-Content -LiteralPath (Join-Path $script:slots 'slot1.lock') -Raw) -eq "$PID`nC:\wt\outer`npass=dddd`nsession=session-a`npreset=mutation") 'a nested claim rewrote the outer claim''s lock'

    # A lock with no preset= line, as a server started before passes named a
    # preset writes, is claimed with any preset.
    foreach ($any in 'default', 'mutation', 'Mutation') {
        New-Case
        $env:HYDRA_BUILD_PASS = 'eeee'
        $env:CLAUDECODE = '1'
        $expires = [DateTime]::UtcNow.AddSeconds(60).ToString('o')
        Write-Lock 1 "$PID`npass for session session-a`npass=eeee`nsession=session-a`nexpires=$expires"
        $r = Invoke-Slot @('-Claim', '-Owner', 'old pass', '-Preset', $any)
        $lock = Read-OnlyLock
        Check ($r.Code -eq 0 -and -not $lock.Unclaimed -and -not $lock.HasPreset) "a lock with no preset line, claimed with ${any}: expected a claim that adds no preset, got exit $($r.Code), Unclaimed $($lock.Unclaimed), HasPreset $($lock.HasPreset):`n$($r.Text)"
    }

    # -Repo under a mutation pass must name mutation, and hands it to the build.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset 'mutation'
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    Check ($r.Code -ne 0 -and $r.Text -cmatch "for the preset 'mutation' only" -and (Get-CMakeLines).Count -eq 0) "-Repo -Preset default under a mutation pass: expected a refusal before configure, got exit $($r.Code):`n$($r.Text)"
    Check ((Read-OnlyLock).Unclaimed) '-Repo -Preset default under a mutation pass: the lock was claimed'
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'mutation', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and $said.Count -ge 1 -and @($said | Where-Object { $_ -notmatch 'locks=1 claimed=1 ' }).Count -eq 0) "-Repo -Preset mutation under a mutation pass: expected one claimed slot, got exit $($r.Code):`n$($r.Text)`n$($said -join "`n")"
    Check (@($said | Where-Object { $_ -cmatch '^--build --preset mutation .*--target hydra_tests( |$)' }).Count -eq 1) "-Repo -Preset mutation: the build did not build the mutation preset:`n$($said -join "`n")"
    Check ((Get-Locks).Count -eq 0) '-Repo -Preset mutation: the slot was not given back'

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
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and $said.Count -eq 3) "-Repo under a pass: exit $($r.Code):`n$($r.Text)"
    Check (@($said | Where-Object { $_ -notmatch 'locks=1 claimed=1 ' }).Count -eq 0) "-Repo under a pass: expected exactly one slot taken:`n$($said -join "`n")"
    Check ((Get-Locks).Count -eq 0) '-Repo under a pass: the slot was not given back'

    New-Case
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'default', '-SlotDir', $script:slots)
    Check ($r.Code -ne 0 -and $r.Text -match 'no build pass' -and (Get-CMakeLines).Count -eq 0) "-Repo with no pass: expected a refusal, got exit $($r.Code):`n$($r.Text)"

    # ------------------------------- a pass names its folder and its targets

    # The reply's command sets the pass and runs this checkout's own
    # build_slot.ps1 on the folder and target asked for; the lock holds both,
    # before and after the claim.
    New-Case
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default', '-Repo', $fakeRepo, '-Target', 'hydra_tests')
    $token = $r.Lines[0] -replace '^pass: ', ''
    $command = "$($r.Lines[2])"
    Check ($r.Code -eq 0 -and $command.StartsWith("`$env:HYDRA_BUILD_PASS='$token'; ")) "a pass for a folder: the command does not set HYDRA_BUILD_PASS first:`n$($r.Text)"
    Check ($command.Contains("& '$slotScript' -Repo '$fakeRepo' -Preset default -Target hydra_tests")) "a pass for a folder: the command does not run this checkout's build_slot.ps1 on the folder and target:`n$($r.Text)"
    $lock = Read-OnlyLock
    Check ($lock.HasRepo -and $lock.Repo -eq $fakeRepo -and (Test-SameSet $lock.Targets @('hydra_tests')) -and $lock.Unclaimed) "a pass for a folder: the unclaimed lock does not hold the folder and target (HasRepo $($lock.HasRepo), '$($lock.Repo)', '$($lock.Targets -join ',')')"
    $env:HYDRA_BUILD_PASS = $token
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Claim', '-Owner', 'claimer', '-Preset', 'default')
    $lock = Read-OnlyLock
    Check ($r.Code -eq 0 -and -not $lock.Unclaimed -and $lock.HasRepo -and $lock.Repo -eq $fakeRepo -and (Test-SameSet $lock.Targets @('hydra_tests'))) "a pass for a folder: after the claim the lock lost its folder or target (Unclaimed $($lock.Unclaimed), '$($lock.Repo)', '$($lock.Targets -join ',')'):`n$($r.Text)"

    # Run as the reply prints it, in one PowerShell process, the command builds
    # under its pass and writes one record line.
    New-Case
    $before = (Get-Records $fakeRepo).Count
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default', '-Repo', $fakeRepo, '-Target', 'hydra_tests')
    $env:CLAUDECODE = '1'
    $printed = Join-Path $fixture 'printed.ps1'
    Set-Content $printed "$($r.Lines[2])`nexit `$LASTEXITCODE"
    $run = Invoke-Pwsh $printed
    Check ($run.Code -eq 0 -and @(Get-CMakeLines | Where-Object { Test-TargetBuildLine $_ }).Count -eq 1 -and (Get-Records $fakeRepo).Count -eq $before + 1) "the printed command: expected one build of hydra_tests and one record, got exit $($run.Code):`n$($run.Text)"
    Check ((Get-Locks).Count -eq 0) 'the printed command: the slot was not given back'

    # A pass with neither is handed out as before, with the old command; one
    # that names only one of the two is refused and takes no slot.
    New-Case
    $r = Invoke-Slot @('-NewPass', '-Session', 'session-a', '-Preset', 'default')
    Check ($r.Code -eq 0 -and $r.Lines[2] -match '\.\\build_cpp\.ps1 -Preset default' -and -not (Read-OnlyLock).HasRepo) "a pass with no folder: expected the old command and a lock with no folder, got exit $($r.Code):`n$($r.Text)"
    New-Case
    foreach ($half in @('-Repo', $fakeRepo), @('-Target', 'hydra_tests'), @('-Repo', $fakeRepo, '-Target', ','), @('-Repo', $fakeRepo, '-Target', 'two words')) {
        $r = Invoke-Slot (@('-NewPass', '-Session', 'session-a', '-Preset', 'default') + $half)
        Check ($r.Code -eq 1 -and $r.Text -match '^no pass: ') "a pass naming '$($half -join ' ')': expected a refusal, got exit $($r.Code):`n$($r.Text)"
    }
    Check ((Get-Locks).Count -eq 0) 'a refused pass for half a folder and target left a lock'

    # Two targets: the pass builds both, in one build, and refuses a build of
    # one of them, or of another folder, before anything runs or is recorded.
    New-Case
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Repo $fakeRepo -Target 'a,b'
    $env:CLAUDECODE = '1'
    $before = (Get-Records $fakeRepo).Count
    $elsewhere = Join-Path $fixture 'elsewhere'
    foreach ($wrong in @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'a'), @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'a,b,c'), @('-Repo', $elsewhere, '-Preset', 'default', '-Target', 'a,b')) {
        $r = Invoke-Slot ($wrong + @('-SlotDir', $script:slots))
        Check ($r.Code -ne 0 -and $r.Text -match 'this build pass is for the (folder|targets)' -and (Get-CMakeLines).Count -eq 0) "-Repo $($wrong -join ' ') under a pass for a,b: expected a refusal before configure, got exit $($r.Code):`n$($r.Text)`n$((Get-CMakeLines) -join "`n")"
        Check ((Get-Records $fakeRepo).Count -eq $before) "-Repo $($wrong -join ' ') under a pass for a,b: a refused build wrote a record line"
        Check ((Read-OnlyLock).Unclaimed) "-Repo $($wrong -join ' ') under a pass for a,b: the lock was claimed"
    }
    # The same folder spelled with other slashes and case, and the targets in
    # another order, is the folder and targets of the pass.
    $respelled = $fakeRepo.ToUpperInvariant().Replace('\', '/') + '/'
    $r = Invoke-Slot @('-Repo', $respelled, '-Preset', 'default', '-Target', 'b,a', '-SlotDir', $script:slots)
    $said = Get-CMakeLines
    Check ($r.Code -eq 0 -and @($said | Where-Object { $_ -match '^--build .*--target b --target a( |$)' }).Count -eq 1) "-Repo b,a under a pass for a,b: expected one build of both, got exit $($r.Code):`n$($r.Text)`n$($said -join "`n")"
    $records = @(Get-Records $fakeRepo)
    Check ($records.Count -eq $before + 1 -and (Test-SameSet $records[-1].targets @('b', 'a'))) "-Repo b,a: the record does not list the targets built: $($records[-1] | ConvertTo-Json -Compress)"

    # ------------------------------------------------------ the build record

    # One line per build, passing or failing, each field what git and the
    # command line say at the build's start.
    foreach ($fail in '', '3') {
        New-Case
        Set-Content (Join-Path $fakeRepo 'notes.txt') "changed $fail"
        Set-Content (Join-Path $fakeRepo "untracked$fail.txt") 'new'
        $head = "$(Invoke-Git $fakeRepo @('rev-parse', 'HEAD'))".Trim()
        $dirty = Get-PorcelainPaths $fakeRepo
        $before = (Get-Records $fakeRepo).Count
        $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Repo $fakeRepo -Target 'hydra_tests'
        $env:CLAUDECODE = '1'
        $env:FAKE_CMAKE_EXIT = $fail
        $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
        $label = if ($fail) { 'failing' } else { 'passing' }
        Check (($r.Code -eq 0) -eq (-not $fail)) "record, $label build: unexpected exit $($r.Code):`n$($r.Text)"
        $records = @(Get-Records $fakeRepo)
        Check ($records.Count -eq $before + 1) "record, $label build: expected one new line, got $($records.Count - $before)"
        $rec = $records[-1]
        $raw = (Get-Content -LiteralPath (Join-Path $fakeRepo '.git\build_results.jsonl'))[-1] | ConvertFrom-Json -AsHashtable
        Check ((Test-SameSet $raw.Keys @('repo', 'commit', 'dirty', 'preset', 'targets', 'build_dir', 'exit_code', 'switched_copies', 'time'))) "record, $label build: the fields are $($raw.Keys -join ',')"
        Check ($rec.repo -eq $fakeRepo -and $rec.commit -eq $head -and $rec.preset -ceq 'default') "record, $label build: repo, commit or preset is wrong: $($rec | ConvertTo-Json -Compress)"
        Check ($dirty.Count -eq 2 -and $raw.dirty -is [System.Collections.IList] -and (Test-SameSet $rec.dirty $dirty)) "record, $label build: dirty is $($rec.dirty -join ','), git status said $($dirty -join ',')"
        Check ($raw.targets -is [System.Collections.IList] -and (Test-SameSet $rec.targets @('hydra_tests'))) "record, $label build: targets is $($rec.targets -join ',')"
        Check ($raw.exit_code -is [long] -or $raw.exit_code -is [int]) "record, $label build: exit_code is not a number"
        Check ($rec.exit_code -eq $r.Code -and (($rec.exit_code -eq 0) -eq (-not $fail))) "record, $label build: exit_code $($rec.exit_code), the build exited $($r.Code)"
        Check ($raw.switched_copies -is [bool]) "record, $label build: switched_copies is not true or false"
        # ConvertFrom-Json turns an ISO 8601 string into a date by itself.
        Check ($raw.time -is [DateTime]) "record, $label build: time '$($raw.time)' is not an ISO date"
        Remove-Item -LiteralPath (Join-Path $fakeRepo "untracked$fail.txt")
        Invoke-Git $fakeRepo @('checkout', '--', 'notes.txt') | Out-Null
    }

    # Dirty paths are plain paths as git names them: no status letters, no
    # quotes around a name with a space, and a rename as its new path and its
    # old path, each its own entry.
    New-Case
    Set-Content (Join-Path $fakeRepo 'with space.txt') 'new'
    Invoke-Git $fakeRepo @('mv', 'notes.txt', 'renamed notes.txt') | Out-Null
    $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', 'default', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $rec = @(Get-Records $fakeRepo)[-1]
    Check ($r.Code -eq 0 -and (Test-SameSet $rec.dirty @('with space.txt', 'renamed notes.txt', 'notes.txt'))) "dirty with a space and a rename: expected 'with space.txt', 'renamed notes.txt' and 'notes.txt', got $($rec.dirty | ConvertTo-Json -Compress)"
    Remove-Item -LiteralPath (Join-Path $fakeRepo 'with space.txt')
    Invoke-Git $fakeRepo @('mv', 'renamed notes.txt', 'notes.txt') | Out-Null
    Check ((Get-PorcelainPaths $fakeRepo).Count -eq 0) 'dirty with a space and a rename: the fixture repo was not put back clean'

    # A build's own exit code is the record's, as the stand-in build gives it.
    New-Case
    $codeRepo = Join-Path $fixture 'code_repo'
    New-Item -ItemType Directory -Force $codeRepo | Out-Null
    Set-Content (Join-Path $codeRepo 'build_cpp.ps1') "& '$fakeCMake' coded-build; exit 7"
    Copy-Item (Join-Path $repoRoot 'CMakePresets.json') $codeRepo
    Invoke-Git $codeRepo @('init', '-q', '-b', 'main') | Out-Null
    Invoke-Git $codeRepo @('add', '-A') | Out-Null
    Invoke-Git $codeRepo @('commit', '-q', '-m', 'fixture') | Out-Null
    $r = Invoke-Slot @('-Repo', $codeRepo, '-Preset', 'default', '-Target', 'hydra_tests', '-SlotDir', $script:slots)
    $records = @(Get-Records $codeRepo)
    Check ($r.Code -eq 7 -and $records.Count -eq 1 -and $records[0].exit_code -eq 7) "a build exiting 7: expected exit 7 and one record saying 7, got exit $($r.Code), records $($records | ConvertTo-Json -Compress)"

    # Switched copies: true for the preset whose cache turns HYDRA_MUTATION on,
    # false for default; the build folder is the preset's own.
    $realPresets = (Get-Content -LiteralPath (Join-Path $repoRoot 'CMakePresets.json') -Raw | ConvertFrom-Json).configurePresets
    $mutationCache = ($realPresets | Where-Object { $_.name -eq 'mutation' }).cacheVariables
    Check ($mutationCache.PSObject.Properties['HYDRA_MUTATION'] -and $mutationCache.HYDRA_MUTATION.value -eq 'ON') 'the real CMakePresets.json no longer turns HYDRA_MUTATION on in the mutation preset, so this check reads nothing'
    . (Join-Path $PSScriptRoot 'cmake_presets.ps1')
    foreach ($case in @{ Preset = 'mutation'; Switched = $true }, @{ Preset = 'default'; Switched = $false }) {
        New-Case
        $env:HYDRA_BUILD_PASS = New-Pass 'session-a' -Preset $case.Preset -Repo $fakeRepo -Target 'hydra_tests'
        $env:CLAUDECODE = '1'
        $r = Invoke-Slot @('-Repo', $fakeRepo, '-Preset', $case.Preset, '-Target', 'hydra_tests', '-SlotDir', $script:slots)
        $rec = @(Get-Records $fakeRepo)[-1]
        Check ($r.Code -eq 0 -and $rec.preset -ceq $case.Preset -and $rec.switched_copies -eq $case.Switched) "record, $($case.Preset): expected switched_copies $($case.Switched), got exit $($r.Code), $($rec | ConvertTo-Json -Compress)"
        Check ($rec.build_dir -eq (Get-PresetBuildDir $fakeRepo $case.Preset)) "record, $($case.Preset): build_dir '$($rec.build_dir)' is not the preset's"
    }

    # An old release's build_cpp.ps1 claims nothing, so -Repo's own claim is
    # what holds the slot through its build. Its exit code comes through.
    New-Case
    $oldRepo = Join-Path $fixture 'old_release'
    New-Item -ItemType Directory -Force $oldRepo | Out-Null
    Set-Content (Join-Path $oldRepo 'build_cpp.ps1') "& '$fakeCMake' old-build; exit 5"
    $env:HYDRA_BUILD_PASS = New-Pass 'session-a'
    $env:CLAUDECODE = '1'
    $r = Invoke-Slot @('-Repo', $oldRepo, '-Preset', 'default', '-SlotDir', $script:slots)
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
        $r = Invoke-Slot @('-NewPass', '-Session', 'young', '-Preset', 'default', '-Holder', "$PID")
        Check ($r.Code -eq 1) "a young empty lock was cleared, and a pass given:`n$($r.Text)"
        Check (Test-Path (Join-Path $script:slots 'slot3.lock')) 'a young empty lock was deleted'
        (Get-Item (Join-Path $script:slots 'slot3.lock')).LastWriteTimeUtc = [DateTime]::UtcNow.AddMinutes(-1)
        $r = Invoke-Slot @('-NewPass', '-Session', 'young', '-Preset', 'default', '-Holder', "$PID")
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
