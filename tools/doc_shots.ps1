# Regenerates the pictures in docs/images from the testdata/input corpus.
# Usage: pwsh -NoProfile -File tools\doc_shots.ps1
# The shot list is tools\doc_shots\shots.txt; docs\agents\doc-screenshots.md
# explains the whole step. The runner uses its own scratch library and
# settings, so this script never names a library file or a settings file.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'build-cpp\Release\hydra_uitest.exe'
$shots = Join-Path $root 'tools\doc_shots\shots.txt'
$images = Join-Path $root 'docs\images'
$shrink = Join-Path $root 'tools\doc_shots\shrink.py'

if (-not (Test-Path -LiteralPath $exe)) {
    Write-Error "doc_shots: $exe is missing; build it with .\build_cpp.ps1 -Target hydra_uitest"
    exit 1
}

$output = & $exe --script $shots --shots $images 2>&1 | ForEach-Object { "$_" }
$code = $LASTEXITCODE
if ($code -ne 0) {
    $output | ForEach-Object { Write-Host $_ }
    Write-Error "doc_shots: hydra_uitest exited with $code; its output is above"
    exit $code
}
$output | Where-Object { $_ -match '^screenshot: ' } | ForEach-Object { Write-Host $_ }

# A rect line directly before a screenshot line with no bar keeps only the
# frame below that rectangle (the comment at the top of shots.txt). The k-th
# rect command prints the k-th rect line of the output.
$commands = Get-Content -LiteralPath $shots |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -ne '' -and -not $_.StartsWith('#') }
$rects = @($output | Where-Object { $_ -match ' rect=\(' })
$below = @()
$rect_index = -1
for ($i = 0; $i -lt $commands.Count; $i++) {
    if ($commands[$i] -match '^rect\s') { $rect_index++; continue }
    if ($i -gt 0 -and $commands[$i - 1] -match '^rect\s' -and $commands[$i] -match '^screenshot\s+(\S+)\s*$') {
        $file = $Matches[1]
        if ($rects[$rect_index] -notmatch ' rect=\((-?\d+),(-?\d+) (\d+)x(\d+)\)') {
            Write-Error "doc_shots: no rectangle printed for the rect line before screenshot $file"
            exit 1
        }
        $bottom = [int]$Matches[2] + [int]$Matches[4]
        $below += @('--below', "$file=$bottom")
    }
}

& py -I $shrink $images @below
if ($LASTEXITCODE -ne 0) {
    Write-Error "doc_shots: shrink.py failed; its output is above"
    exit $LASTEXITCODE
}
