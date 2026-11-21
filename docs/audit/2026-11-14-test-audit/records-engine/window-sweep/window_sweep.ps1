param([string]$Repo, [string]$Out)
$R = 'C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'
New-Item -ItemType Directory -Force $Out | Out-Null
$sets = @(
  @{ name = 'default'; args = @() },
  @{ name = 'gui'; args = @('--cap', '4', '--ms', '10', '--depth', '2') }
)
$rows = @("chart`tsettings`tsame`tpaths85`tpaths60`tdetail")
$charts = Get-ChildItem -Recurse (Join-Path $Repo 'testdata\input') -File | Where-Object { $_.Name -match '\.(chart|mid)$' } | Sort-Object FullName
foreach ($c in $charts) {
  $rel = $c.FullName.Substring($Repo.Length + 1).Replace('\', '/')
  foreach ($s in $sets) {
    $j = @{}
    foreach ($w in 85, 60) {
      $txt = (& $R dump --chart $c.FullName @($s.args) --efill $w 2>&1) -join "`n"
      $j[$w] = $txt
    }
    $strip = { param($t) ($t -split "`n" | Where-Object { $_ -notmatch '"hydra_replay_build"' }) -join "`n" }
    $a = & $strip $j[85]; $b = & $strip $j[60]
    $same = ($a -eq $b)
    $pa = ''; $pb = ''; $detail = ''
    try {
      $oa = $j[85] | ConvertFrom-Json; $ob = $j[60] | ConvertFrom-Json
      $pa = ($oa.paths | ForEach-Object { "$($_.pathstring)=$($_.total)" }) -join ' | '
      $pb = ($ob.paths | ForEach-Object { "$($_.pathstring)=$($_.total)" }) -join ' | '
    } catch { $detail = 'not json: ' + (($j[60] -split "`n")[0]) }
    if (-not $same) {
      $safe = ($rel -replace '[^\w.-]', '_')
      Set-Content -Path (Join-Path $Out "$safe.$($s.name).efill85.dump.json") -Value $j[85] -Encoding utf8
      Set-Content -Path (Join-Path $Out "$safe.$($s.name).efill60.dump.json") -Value $j[60] -Encoding utf8
      if (-not $detail) { $detail = if ($pa -eq $pb) { 'paths and totals same; other fields differ' } else { 'paths differ' } }
    }
    $rows += "$rel`t$($s.name)`t$same`t$pa`t$pb`t$detail"
  }
}
Set-Content -Path (Join-Path $Out 'sweep.tsv') -Value $rows -Encoding utf8
$diff = $rows | Select-Object -Skip 1 | Where-Object { ($_ -split "`t")[2] -eq 'False' }
"charts: $($charts.Count); runs: $($rows.Count - 1); differing: $($diff.Count)"
$diff | ForEach-Object { $f = $_ -split "`t"; "$($f[0]) [$($f[1])] $($f[5])" }
