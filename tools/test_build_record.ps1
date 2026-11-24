#Requires -Version 7.0
<#
Self-test for the build record: tools/build_record.ps1, and the two calls of it
in build_cpp.ps1.

    pwsh -NoProfile -File tools/test_build_record.ps1

It compiles nothing and takes no real slot. The fixture is a temporary git
repo holding a copy of the real build_cpp.ps1, build_slot.ps1,
cmake_presets.ps1, stale_toolchain.ps1, build_record.ps1 and CMakePresets.json,
with a find_cmake.ps1 that hands back a fake cmake. The fake writes down its
arguments, outside the repo, and its real build step (not the configure or
ZERO_CHECK) exits with FAKE_CMAKE_EXIT.

Every expected value is read from git or from the fake cmake's own log, never
worked out here. Whether a real build reaches the record is the main
session's check (the plan's live step LS2).
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$repoRoot = Split-Path $PSScriptRoot

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

$fixture = Join-Path ([IO.Path]::GetTempPath()) ("build_record_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))

function Invoke-Pwsh([string]$Script, [string[]]$ScriptArgs = @()) {
    $out = @(& pwsh -NoProfile -File $Script @ScriptArgs 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $LASTEXITCODE; Lines = $out; Text = ($out -join "`n") }
}

function Invoke-FixtureGit([string]$Dir, [string[]]$GitArgs) {
    $out = & git -C $Dir -c user.name=t -c user.email=t@example.invalid @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' ') failed in the fixture: $out" }
    @($out | ForEach-Object { "$_" })
}

function Invoke-Build([string[]]$BuildArgs = @('-Target', 'hydra_tests')) {
    Set-Content $cmakeLog -Value @() -NoNewline
    Invoke-Pwsh $fakeRepoBuild $BuildArgs
}

# The record's lines, read back as objects. An empty or missing file is none.
function Get-Records {
    if (-not (Test-Path -LiteralPath $recordFile -PathType Leaf)) { return @() }
    @(Get-Content -LiteralPath $recordFile | Where-Object { $_ -ne '' } | ForEach-Object { $_ | ConvertFrom-Json -DateKind String })
}

# The lines a build added since $Before of them were there.
function Get-NewRecords([int]$Before) { @(Get-Records | Select-Object -Skip $Before) }

# The targets the fake cmake was asked to build, from its build line.
function Get-BuiltTargets {
    $line = @(Get-Content -LiteralPath $cmakeLog | Where-Object { $_ -match '^--build ' -and $_ -notmatch 'ZERO_CHECK' })
    if ($line.Count -ne 1) { return $null }
    $words = ($line[0] -split ' \| ')[0] -split ' '
    $names = [System.Collections.Generic.List[string]]::new()
    $inTarget = $false
    foreach ($w in $words) {
        if ($w -eq '--target') { $inTarget = $true; continue }
        if ($w -like '-*') { $inTarget = $false; continue }
        if ($inTarget) { $names.Add($w) }
    }
    , $names.ToArray()
}

# git's first output line, as one string.
function Get-GitLine([string]$Dir, [string[]]$GitArgs) { "$(@(Invoke-FixtureGit $Dir $GitArgs)[0])".Trim() }
function Get-Head { Get-GitLine $fakeRepo @('rev-parse', 'HEAD') }
function Get-HeadTree { Get-GitLine $fakeRepo @('rev-parse', 'HEAD^{tree}') }

# Checks a build's two lines: a started line then $EndEvent, both for this
# build, this commit and this tree, with the preset and the targets cmake was
# asked for.
function Test-BuildPair($New, [string]$EndEvent, [bool]$Dirty, [string]$Preset, [string]$Label) {
    $dump = ($New | ForEach-Object { $_ | ConvertTo-Json -Compress }) -join "`n"
    Check ($New.Count -eq 2) "${Label}: expected two record lines, got $($New.Count):`n$dump"
    if ($New.Count -ne 2) { return }
    Check ($New[0].event -eq 'started' -and $New[1].event -eq $EndEvent) "${Label}: expected started then $EndEvent, got:`n$dump"
    Check ($New[0].build -and $New[0].build -eq $New[1].build) "${Label}: the two lines do not name one build:`n$dump"
    $head = Get-Head
    $tree = Get-HeadTree
    $built = Get-BuiltTargets
    foreach ($r in $New) {
        Check ($r.kind -eq 'build') "${Label}: kind is '$($r.kind)'"
        Check ($r.commit -eq $head -and $r.tree -eq $tree) "${Label}: expected commit $head tree $tree, got $($r.commit) $($r.tree)"
        Check ($r.dirty -eq $Dirty) "${Label}: expected dirty $Dirty, got $($r.dirty)"
        Check ($r.preset -eq $Preset) "${Label}: expected preset $Preset, got $($r.preset)"
        Check ($null -ne $built -and (@($r.targets) -join ',') -eq ($built -join ',')) "${Label}: the record names targets '$(@($r.targets) -join ',')', cmake was asked for '$($built -join ',')'"
        Check ($r.at -match '^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d[+-]\d\d:\d\d$') "${Label}: the time '$($r.at)' is not a local time with its offset"
    }
}

$callerEnv = @{}
foreach ($name in 'CLAUDECODE', 'HYDRA_BUILD_PASS', 'HYDRA_BUILD_SLOT_DIR', 'FAKE_CMAKE_EXIT', 'FAKE_CMAKE_LOG', 'PATH') { $callerEnv[$name] = [Environment]::GetEnvironmentVariable($name) }

try {
    # ------------------------------------------------------------ the fixture
    $fakeRepo = Join-Path $fixture 'repo'
    $fakeRepoBuild = Join-Path $fakeRepo 'build_cpp.ps1'
    $cmakeLog = Join-Path $fixture 'cmake.log'
    New-Item -ItemType Directory -Force -Path (Join-Path $fakeRepo 'tools') | Out-Null
    Copy-Item (Join-Path $repoRoot 'build_cpp.ps1'), (Join-Path $repoRoot 'CMakePresets.json') $fakeRepo
    Copy-Item (Join-Path $PSScriptRoot 'build_slot.ps1'), (Join-Path $PSScriptRoot 'cmake_presets.ps1'), (Join-Path $PSScriptRoot 'stale_toolchain.ps1'), (Join-Path $PSScriptRoot 'build_record.ps1') (Join-Path $fakeRepo 'tools')
    $fakeCMake = Join-Path $fixture 'fake_cmake.ps1'
    Set-Content (Join-Path $fakeRepo 'tools\find_cmake.ps1') "function Find-CMake { '$fakeCMake' }"
    Set-Content $fakeCMake @'
Add-Content -LiteralPath $env:FAKE_CMAKE_LOG "$($args -join ' ') | "
$realBuild = $args -contains '--build' -and $args -notcontains 'ZERO_CHECK'
if ($realBuild -and $env:FAKE_CMAKE_EXIT) { exit ([int]$env:FAKE_CMAKE_EXIT) }
exit 0
'@
    Set-Content (Join-Path $fakeRepo 'notes.txt') 'committed'
    Invoke-FixtureGit $fakeRepo @('init', '-q', '-b', 'main') | Out-Null
    Invoke-FixtureGit $fakeRepo @('add', '-A') | Out-Null
    Invoke-FixtureGit $fakeRepo @('commit', '-q', '-m', 'fixture') | Out-Null
    $common = Get-GitLine $fakeRepo @('rev-parse', '--path-format=absolute', '--git-common-dir')
    $recordFile = Join-Path $common 'builds.jsonl'

    $env:FAKE_CMAKE_LOG = $cmakeLog
    $env:HYDRA_BUILD_SLOT_DIR = Join-Path $fixture 'slots'
    New-Item -ItemType Directory -Path $env:HYDRA_BUILD_SLOT_DIR | Out-Null
    Remove-Item Env:CLAUDECODE, Env:HYDRA_BUILD_PASS, Env:FAKE_CMAKE_EXIT -ErrorAction SilentlyContinue
    # Only here so build_cpp.ps1 skips entering a Visual Studio shell for a
    # Ninja preset; nothing runs it.
    Set-Content (Join-Path $fixture 'cl.exe') ''
    $env:PATH = "$fixture;$env:PATH"

    # --------------------------------------------------- a passing build
    $before = (Get-Records).Count
    $r = Invoke-Build
    Check ($r.Code -eq 0) "passing build: exit $($r.Code):`n$($r.Text)"
    Test-BuildPair (Get-NewRecords $before) 'passed' $false 'default' 'passing build'
    Check ($r.Text -notmatch 'build record:') "passing build: a record error was printed:`n$($r.Text)"

    # --------------------------------------------------- a build that threw
    $before = (Get-Records).Count
    $env:FAKE_CMAKE_EXIT = '3'
    $r = Invoke-Build
    Remove-Item Env:FAKE_CMAKE_EXIT
    Check ($r.Code -ne 0 -and $r.Text -match 'build failed') "failing build: expected the build's own failure, got exit $($r.Code):`n$($r.Text)"
    Test-BuildPair (Get-NewRecords $before) 'failed' $false 'default' 'failing build'

    # --------------------------------------------------- a plain build, another preset
    $before = (Get-Records).Count
    $r = Invoke-Build @('-Preset', 'coverage-ninja')
    Check ($r.Code -eq 0) "plain Ninja build: exit $($r.Code):`n$($r.Text)"
    Test-BuildPair (Get-NewRecords $before) 'passed' $false 'coverage-ninja' 'plain Ninja build'

    # --------------------------------------------------- an uncommitted change
    $before = (Get-Records).Count
    Set-Content (Join-Path $fakeRepo 'notes.txt') 'changed, not committed'
    $r = Invoke-Build
    Check ($r.Code -eq 0) "dirty build: exit $($r.Code):`n$($r.Text)"
    Test-BuildPair (Get-NewRecords $before) 'passed' $true 'default' 'dirty build'
    Invoke-FixtureGit $fakeRepo @('checkout', '--', 'notes.txt') | Out-Null

    # An untracked file counts too, as the real-app record counts it.
    $before = (Get-Records).Count
    $stray = Join-Path $fakeRepo 'stray.txt'
    Set-Content $stray 'untracked'
    $r = Invoke-Build
    Test-BuildPair (Get-NewRecords $before) 'passed' $true 'default' 'untracked file'
    Remove-Item -LiteralPath $stray

    # --------------------------------------------------- a refused claim
    $before = (Get-Records).Count
    $env:CLAUDECODE = '1'
    $r = Invoke-Build
    Remove-Item Env:CLAUDECODE
    Check ($r.Code -ne 0 -and $r.Text -match 'no build pass') "refused claim: expected the claim's refusal, got exit $($r.Code):`n$($r.Text)"
    Check ((Get-NewRecords $before).Count -eq 0) "refused claim: a record line was written for a build that never started"

    # --------------------------------------------------- a record that cannot be written
    # A folder where the file should be makes every append fail.
    Move-Item -LiteralPath $recordFile -Destination "$recordFile.kept"
    New-Item -ItemType Directory -Path $recordFile | Out-Null
    try {
        $r = Invoke-Build
        Check ($r.Code -eq 0) "unwritable record, passing build: the build's exit changed to $($r.Code):`n$($r.Text)"
        Check ($r.Text -match "build record: the 'started' line was not written" -and $r.Text -match "build record: the 'passed' line was not written") "unwritable record, passing build: no error for each line:`n$($r.Text)"
        Check ($r.Text -match 'Build succeeded') "unwritable record, passing build: the build's own words are missing:`n$($r.Text)"
        Check ($null -ne (Get-BuiltTargets)) "unwritable record, passing build: cmake did not build:`n$($r.Text)"

        $env:FAKE_CMAKE_EXIT = '3'
        $r = Invoke-Build
        Remove-Item Env:FAKE_CMAKE_EXIT
        Check ($r.Code -ne 0 -and $r.Text -match 'build failed') "unwritable record, failing build: the build's failure was hidden, exit $($r.Code):`n$($r.Text)"
        Check ($r.Text -match "build record: the 'failed' line was not written") "unwritable record, failing build: no error for the end line:`n$($r.Text)"
    } finally {
        Remove-Item -LiteralPath $recordFile
        Move-Item -LiteralPath "$recordFile.kept" -Destination $recordFile
    }

    # A checkout with no helper at all: the build still runs and says so.
    $helper = Join-Path $fakeRepo 'tools\build_record.ps1'
    Move-Item -LiteralPath $helper -Destination (Join-Path $fixture 'build_record.ps1')
    try {
        $before = (Get-Records).Count
        $r = Invoke-Build
        Check ($r.Code -eq 0 -and $r.Text -match 'Build succeeded') "no helper: the build's result changed, exit $($r.Code):`n$($r.Text)"
        Check ($r.Text -match "build record: the 'started' line was not written") "no helper: no error was printed:`n$($r.Text)"
        Check ((Get-NewRecords $before).Count -eq 0) 'no helper: a line was written anyway'
    } finally {
        Move-Item -LiteralPath (Join-Path $fixture 'build_record.ps1') -Destination $helper
    }

    # --------------------------------------------------- a worktree shares the file
    $worktree = Join-Path $fixture 'wt'
    Invoke-FixtureGit $fakeRepo @('worktree', 'add', '-q', '-b', 'other', $worktree) | Out-Null
    Set-Content (Join-Path $worktree 'notes.txt') 'on the other branch'
    Invoke-FixtureGit $worktree @('commit', '-q', '-am', 'other') | Out-Null
    $before = (Get-Records).Count
    $r = Invoke-Pwsh (Join-Path $worktree 'tools\build_record.ps1') @('-Repo', $worktree, '-Event', 'started', '-Preset', 'default', '-Targets', 'hydra_tests', '-Build', 'wt-build')
    $new = Get-NewRecords $before
    $wtHead = Get-GitLine $worktree @('rev-parse', 'HEAD')
    Check ($r.Code -eq 0 -and $new.Count -eq 1 -and $new[0].commit -eq $wtHead -and $new[0].build -eq 'wt-build') "worktree: expected one line naming the worktree's commit $wtHead in the shared file, got exit $($r.Code), $($new.Count) lines:`n$($r.Text)"
    Check (-not (Test-Path -LiteralPath (Join-Path $worktree 'builds.jsonl'))) 'worktree: a record file appeared inside the worktree'

    # The helper on its own refuses an event it does not know, and writes nothing.
    $before = (Get-Records).Count
    $r = Invoke-Pwsh $helper @('-Repo', $fakeRepo, '-Event', 'finished', '-Preset', 'default', '-Build', 'x')
    Check ($r.Code -ne 0 -and (Get-NewRecords $before).Count -eq 0) "unknown event: expected a refusal and no line, got exit $($r.Code):`n$($r.Text)"
} finally {
    foreach ($name in $callerEnv.Keys) { [Environment]::SetEnvironmentVariable($name, $callerEnv[$name]) }
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_build_record: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_build_record: PASS, $($script:Passes) checks"
exit 0
