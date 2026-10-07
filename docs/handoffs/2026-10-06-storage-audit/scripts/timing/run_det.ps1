$s='C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\audit2'
$f = Get-Content "$s\det_folders.txt" | ? { $_ }
$t=Get-Date
& "$s\ship\hydra_batch.exe" --redo --db "$s\det\redo.db" @f > "$s\det\batch.txt" 2>&1
"exit $LASTEXITCODE wall $([math]::Round(((Get-Date)-$t).TotalSeconds,1)) s"
Get-Content "$s\det\batch.txt" | Select -Last 4
