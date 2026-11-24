#Requires -Version 7.0
<#
Appends one line to the build record: the file every worktree of one repo
shares, in its git common folder, so git never tracks it. build_cpp.ps1 calls
this once when it has claimed its pass and once when it ends.

    & tools\build_record.ps1 -Repo <checkout> -Event started|passed|failed `
        -Preset <preset> -Targets <names> -Build <id>

This file is the one owner of the record's file name and fields. The real-app
record beside it is tools/hydra_ui_mcp.py's (realapp_record), and the two share
their commit, tree and dirty fields so one reader can compare them.

Each line is one JSON object:
  kind     always "build", as the real-app record's lines say "realapp".
  event    started, passed or failed.
  build    an id the caller makes once per build, so a reader pairs a
           build's started line with its end line.
  commit   the checkout's HEAD at the moment of the line.
  tree     the id of HEAD's tree, git's snapshot of every file the commit holds.
  dirty    true when git status lists anything, untracked files included,
           as the real-app record counts it.
  preset   the build_cpp.ps1 preset.
  targets  the targets cmake was asked to build.
  at       the local time with its UTC offset, to the second.

Which builds count, and for what, is the reader's rule, never this helper's.
It throws when git cannot say or the line cannot be written; the caller
decides what a failed write means for the build (build_cpp.ps1 reports it and
carries on).
#>
param(
    [Parameter(Mandatory)][string]$Repo,
    [Parameter(Mandatory)][ValidateSet('started', 'passed', 'failed')][string]$Event,
    [Parameter(Mandatory)][string]$Preset,
    [string[]]$Targets = @(),
    [Parameter(Mandatory)][string]$Build
)

$ErrorActionPreference = 'Stop'

$BuildRecordFile = 'builds.jsonl'

# git's output for $GitArgs in $Repo, or a throw that carries git's own words.
function Invoke-RecordGit([string[]]$GitArgs) {
    $out = & {
        $ErrorActionPreference = 'Continue'
        & git -C $Repo @GitArgs 2>&1
    }
    $code = $LASTEXITCODE
    $text = @($out | ForEach-Object { "$_" })
    if ($code -ne 0) { throw "git $($GitArgs -join ' ') failed in ${Repo}: $(($text -join ' ').Trim())" }
    $text
}

$common = (Invoke-RecordGit @('rev-parse', '--path-format=absolute', '--git-common-dir') | Select-Object -First 1).Trim()
$commit = (Invoke-RecordGit @('rev-parse', 'HEAD') | Select-Object -First 1).Trim()
$tree = (Invoke-RecordGit @('rev-parse', 'HEAD^{tree}') | Select-Object -First 1).Trim()
$dirty = @(Invoke-RecordGit @('status', '--porcelain') | Where-Object { $_ -ne '' }).Count -gt 0

$record = [ordered]@{
    kind    = 'build'
    event   = $Event
    build   = $Build
    commit  = $commit
    tree    = $tree
    dirty   = $dirty
    preset  = $Preset
    targets = @($Targets)
    at      = (Get-Date).ToString('yyyy-MM-ddTHH:mm:sszzz')
}
$line = ($record | ConvertTo-Json -Compress) + "`n"

# One write of the whole line, opened so a second build appending at the same
# moment is not refused the file.
$path = Join-Path $common $BuildRecordFile
$bytes = [Text.UTF8Encoding]::new($false).GetBytes($line)
$stream = [IO.FileStream]::new($path, [IO.FileMode]::Append, [IO.FileAccess]::Write, [IO.FileShare]::ReadWrite)
try { $stream.Write($bytes, 0, $bytes.Length) } finally { $stream.Dispose() }
