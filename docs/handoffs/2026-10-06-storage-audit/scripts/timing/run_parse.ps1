$b='C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\audit2\bin\hydra_bench.exe'
$t=Get-Date
& $b --parse 'C:\Clone Hero' --out 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\audit2\parse.tsv'
"exit $LASTEXITCODE wall $([math]::Round(((Get-Date)-$t).TotalSeconds,1)) s"
