$b='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_bench.exe'
$t=Get-Date
& $b 'C:\Clone Hero' > 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\audit2\breakdown.txt' 2>&1
"exit $LASTEXITCODE wall $([math]::Round(((Get-Date)-$t).TotalSeconds,1)) s"
