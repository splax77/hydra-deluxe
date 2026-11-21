# V1 B9 item 9: the fill comparison page's chart count, on the 98 corpus charts (never the user's library).
# The main session runs this from a checkout whose build-cpp\Release holds hydra_batch.exe and hydra_fillcompare.exe.
# Usage: pwsh -File v1_b9_fillpage.ps1 -Release <build-cpp\Release folder> -Charts <testdata\input folder> -Work <empty scratch folder>
param([Parameter(Mandatory)][string]$Release, [Parameter(Mandatory)][string]$Charts, [Parameter(Mandatory)][string]$Work)
$ErrorActionPreference = 'Stop'
$db = Join-Path $Work 'both.db'
$html = Join-Path $Work 'fill_compare.html'
& (Join-Path $Release 'hydra_batch.exe') --db $db $Charts
& (Join-Path $Release 'hydra_batch.exe') --db $db --legacy-fills $Charts
& (Join-Path $Release 'hydra_fillcompare.exe') --db $db --out $html --no-open
# The page's own count: the "total" it was built with, and how many rows carry a "k" (one row per chart hash).
Select-String -Path $html -Pattern 'total:' | Select-Object -First 3
(Select-String -Path $html -Pattern '\{"k":' -AllMatches).Matches.Count
