# Lists the single-owner rows by age, oldest first, one line each: the day the
# row was added, the decision behind it, the task that added it, its rules file
# and its question. It is for a review of old rows: a row whose owner is now
# the only way to spell the answer can be retired by hand.
#
# The rows live in tests/single_owner/rules/; which files hold them (select_rule_files)
# and how their fields are read (parse_rule) is decided in
# tests/single_owner/engine.cpp, which this script follows.
#
# From the repo root:
#   pwsh -NoProfile -File tools\single_owner_rows.ps1 [-Before 2026-10-05] [-Area ui]
#
# -Before keeps rows added before that day (YYYY-MM-DD). -Area keeps one rules
# file, named without its folder or .json (ui for rules/ui.json).
# (-Area ui matches a file named ui.json in any rules folder.)

param(
    [string] $Before = '',
    [string] $Area = ''
)

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$rulesDir = Join-Path (Split-Path -Parent $PSScriptRoot) 'tests/single_owner/rules'
if ($Before -ne '' -and $Before -notmatch '^\d{4}-\d{2}-\d{2}$') {
    Write-Host "single_owner_rows: -Before takes a YYYY-MM-DD date, not '$Before'"
    exit 2
}

# The files are the ones select_rule_files in engine.cpp names: .json files in the
# folder and in the folders one level down, each named by its path under
# rules/, in name order (byte by byte, as the scan sorts them).
$files = @(Get-ChildItem -Path $rulesDir -Filter '*.json' -Recurse -Depth 1 -File | ForEach-Object {
    [pscustomobject]@{ Name = ([System.IO.Path]::GetRelativePath($rulesDir, $_.FullName) -replace '\\', '/'); BaseName = $_.BaseName; FullName = $_.FullName }
})
$names = [string[]]@($files | ForEach-Object { $_.Name })
$byName = @{}; foreach ($f in $files) { $byName[$f.Name] = $f }
[System.Array]::Sort($names, [System.StringComparer]::Ordinal)
$files = @($names | ForEach-Object { $byName[$_] })
if ($Area -ne '') {
    $files = @($files | Where-Object { $_.BaseName -eq $Area })
    if ($files.Count -eq 0) {
        Write-Host "single_owner_rows: no rules file named $Area.json in $rulesDir"
        exit 2
    }
}

$rows = foreach ($f in $files) {
    $doc = Get-Content -LiteralPath $f.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
    $index = 0
    foreach ($r in $doc.rules) {
        [pscustomobject]@{
            Date     = $r.date
            Decision = $r.decision
            AddedBy  = $r.added_by
            File     = "rules/$($f.Name)"
            Index    = $index
            Question = $r.question
        }
        $index++
    }
}

if ($Before -ne '') { $rows = @($rows | Where-Object { $_.Date -lt $Before }) }

# ISO dates sort as text; ties keep file order, then row order.
foreach ($r in ($rows | Sort-Object Date, File, Index)) {
    "$($r.Date)  $($r.Decision)  $($r.AddedBy)  $($r.File)  $($r.Question)"
}
