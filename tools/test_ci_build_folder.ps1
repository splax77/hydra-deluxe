#Requires -Version 7.0
<#
Self-test for tools/ci_build_folder.ps1.

    pwsh -NoProfile -File tools/test_ci_build_folder.ps1

It builds a tiny git repository and a fake build folder in a temp folder,
then runs the real script on it through its -Repo parameter, in its own
pwsh each time. The cases: a folder with no record is left alone; a record
at an ancestor commit with one file changed since gives that file "now" and
every other tracked file the script's old time; a range that changes
CMakeLists.txt deletes the folder; a record at a commit that is not an
ancestor deletes the folder; a record naming a commit the clone lacks
deletes the folder. The fixture is deleted at the end; this repository is
never touched.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0
$tool = Join-Path $PSScriptRoot 'ci_build_folder.ps1'

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

$fixture = Join-Path ([IO.Path]::GetTempPath()) ("ci_build_folder_fixture_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
$repo = Join-Path $fixture 'repo'
$build = Join-Path $repo 'build-cpp'
$record = Join-Path $build '.ci-built-commit'
New-Item -ItemType Directory -Path $repo | Out-Null

function Invoke-FixtureGit([string[]]$GitArgs) {
    $out = & git -C $repo -c user.name=probe -c user.email=probe@example.invalid -c core.autocrlf=false @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' '): $($out | Out-String)" }
    $out
}
function Write-Text([string]$Rel, [string]$Text) {
    $p = Join-Path $repo $Rel
    New-Item -ItemType Directory -Force -Path (Split-Path $p) | Out-Null
    [IO.File]::WriteAllText($p, $Text)
}
function Commit([string]$Msg) {
    Invoke-FixtureGit @('add', '-A') | Out-Null
    Invoke-FixtureGit @('commit', '-q', '-m', $Msg) | Out-Null
    "$(Invoke-FixtureGit @('rev-parse', 'HEAD'))".Trim()
}
function New-BuildFolder {
    if (Test-Path $build) { Remove-Item -Recurse -Force $build }
    New-Item -ItemType Directory -Path $build | Out-Null
    [IO.File]::WriteAllText((Join-Path $build 'object.obj'), 'fake object')
}
function Invoke-Tool([string]$Mode) {
    $out = @(& pwsh -NoProfile -File $tool $Mode -Repo $repo 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Code = $LASTEXITCODE; Text = ($out -join "`n") }
}
function Time-Of([string]$Rel) { [IO.File]::GetLastWriteTimeUtc((Join-Path $repo $Rel)) }

$old = [datetime]::new(2000, 1, 1, 0, 0, 0, [DateTimeKind]::Utc)
$sentinel = [datetime]::new(2010, 6, 1, 0, 0, 0, [DateTimeKind]::Utc)

try {
    Write-Text '.gitignore' "build-cpp/`n"
    Write-Text 'CMakeLists.txt' "project(fixture)`n"
    Write-Text 'src/a.cpp' "int a() { return 1; }`n"
    Write-Text 'src/b.cpp' "int b() { return 2; }`n"
    Write-Text 'docs/notes.md' "notes`n"
    Invoke-FixtureGit @('init', '-q', '-b', 'main') | Out-Null
    $c1 = Commit 'first'

    # Case 1: no record. The folder and the file times stay as they were.
    New-BuildFolder
    [IO.File]::SetLastWriteTimeUtc((Join-Path $repo 'src/a.cpp'), $sentinel)
    $r = Invoke-Tool '-Prepare'
    Check ($r.Code -eq 0) "no record: exit $($r.Code):`n$($r.Text)"
    Check ((Test-Path (Join-Path $build 'object.obj')) -and -not (Test-Path $record)) "no record: the folder was changed:`n$($r.Text)"
    Check ((Time-Of 'src/a.cpp') -eq $sentinel) "no record: src/a.cpp's time changed to $(Time-Of 'src/a.cpp')"
    Check ($r.Text -match 'cold build') "no record: no 'cold build' line:`n$($r.Text)"

    # -Record writes HEAD's full id.
    $r = Invoke-Tool '-Record'
    Check ($r.Code -eq 0 -and (Test-Path $record) -and ([IO.File]::ReadAllText($record).Trim() -eq $c1)) "record: expected $c1 in the record, got exit $($r.Code):`n$($r.Text)"

    # Case 2: an ancestor record and one changed file.
    Write-Text 'src/a.cpp' "int a() { return 3; }`n"
    $c2 = Commit 'change a'
    $before = [datetime]::UtcNow.AddSeconds(-5)
    $r = Invoke-Tool '-Prepare'
    Check ($r.Code -eq 0) "ancestor: exit $($r.Code):`n$($r.Text)"
    Check (Test-Path (Join-Path $build 'object.obj')) "ancestor: the folder was deleted:`n$($r.Text)"
    Check ((Time-Of 'src/a.cpp') -ge $before) "ancestor: src/a.cpp is not 'now' ($(Time-Of 'src/a.cpp'))"
    foreach ($f in '.gitignore', 'CMakeLists.txt', 'src/b.cpp', 'docs/notes.md') {
        Check ((Time-Of $f) -eq $old) "ancestor: $f has $(Time-Of $f), not 2000-01-01"
    }
    Check ($r.Text -match 'warm build') "ancestor: no 'warm build' line:`n$($r.Text)"

    # Case 3: a range that changes CMakeLists.txt.
    $null = Invoke-Tool '-Record'
    Write-Text 'CMakeLists.txt' "project(fixture2)`n"
    $null = Commit 'change the build'
    $r = Invoke-Tool '-Prepare'
    Check ($r.Code -eq 0 -and -not (Test-Path $build)) "build file: expected the folder deleted, got exit $($r.Code):`n$($r.Text)"
    Check ($r.Text -match 'CMakeLists\.txt') "build file: the line does not name CMakeLists.txt:`n$($r.Text)"

    # Case 4: a record at a commit that is not an ancestor (another branch).
    Invoke-FixtureGit @('checkout', '-q', '-b', 'side', $c2) | Out-Null
    Write-Text 'src/b.cpp' "int b() { return 4; }`n"
    $side = Commit 'side change'
    Invoke-FixtureGit @('checkout', '-q', 'main') | Out-Null
    New-BuildFolder
    [IO.File]::WriteAllText($record, "$side`n")
    $r = Invoke-Tool '-Prepare'
    Check ($r.Code -eq 0 -and -not (Test-Path $build)) "not an ancestor: expected the folder deleted, got exit $($r.Code):`n$($r.Text)"
    Check ($r.Text -match 'not an ancestor') "not an ancestor: no reason given:`n$($r.Text)"

    # Case 5: a record naming a commit the clone does not have.
    New-BuildFolder
    [IO.File]::WriteAllText($record, ('0123456789abcdef0123456789abcdef01234567' + "`n"))
    $r = Invoke-Tool '-Prepare'
    Check ($r.Code -eq 0 -and -not (Test-Path $build)) "missing commit: expected the folder deleted, got exit $($r.Code):`n$($r.Text)"
    Check ($r.Text -match 'not in this clone') "missing commit: no reason given:`n$($r.Text)"
} finally {
    Remove-Item -Recurse -Force -LiteralPath $fixture -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL $_" }
    Write-Host "test_ci_build_folder: FAILED, $($script:Failures.Count) failures, $($script:Passes) passes"
    exit 1
}
Write-Host "test_ci_build_folder: PASS, $($script:Passes) checks"
exit 0
