#Requires -Version 7.0
<#
Self-test for tools/stale_toolchain.ps1, which tells build_cpp.ps1 when a build
folder's saved toolchain is gone.

    pwsh -NoProfile -File tools/test_stale_toolchain.ps1

It builds fake build folders in a temporary fixture and runs no cmake. Each
case names a tool path that exists (a file in the fixture) or one that does not.
Whether --fresh then clears a real folder needs a real configure, so the main
session checks that.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'stale_toolchain.ps1')
$script:Failures = [System.Collections.Generic.List[string]]::new()
$script:Passes = 0

function Check([bool]$Ok, [string]$What) {
    if ($Ok) { $script:Passes++ } else { $script:Failures.Add($What) }
}

$fixture = Join-Path ([IO.Path]::GetTempPath()) ("stale_toolchain_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $fixture | Out-Null
$vs = Join-Path $fixture 'VS\18\Community'
$tools = Join-Path $vs 'VC\Tools\MSVC\14.52.37025\bin\Hostx64\x64'
New-Item -ItemType Directory -Path $tools | Out-Null
foreach ($exe in 'cl.exe', 'link.exe', 'lib.exe') { New-Item -ItemType File -Path (Join-Path $tools $exe) | Out-Null }
$gone = Join-Path $vs 'VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64'
$fwd = { param($p) $p -replace '\\', '/' }
$installs = @{ ([IO.Path]::GetFullPath($vs)).TrimEnd('\').ToLowerInvariant() = '18.12.11310.141' }

# A build folder with the given cache lines and, when -Compiler is set, a
# CMakeCCompiler.cmake naming that compiler.
function New-BuildDir([string]$Name, [string[]]$CacheLines, [string]$Compiler = '') {
    $dir = Join-Path $fixture $Name
    New-Item -ItemType Directory -Path (Join-Path $dir 'CMakeFiles\4.3.1-msvc1') -Force | Out-Null
    Set-Content -LiteralPath (Join-Path $dir 'CMakeCache.txt') -Value $CacheLines
    if ($Compiler) {
        Set-Content -LiteralPath (Join-Path $dir 'CMakeFiles\4.3.1-msvc1\CMakeCCompiler.cmake') -Value "set(CMAKE_C_COMPILER `"$Compiler`")"
    }
    $dir
}

try {
    $okInstance = "CMAKE_GENERATOR_INSTANCE:INTERNAL=$(& $fwd $vs)"
    $okLinker = "CMAKE_LINKER:FILEPATH=$(& $fwd (Join-Path $tools 'link.exe'))"

    $dir = New-BuildDir 'healthy' @($okInstance, $okLinker, 'CMAKE_RC_COMPILER:FILEPATH=rc') (& $fwd (Join-Path $tools 'cl.exe'))
    Check ($null -eq (Get-StaleToolchainReason $dir $installs)) 'a folder whose saved tools all exist is not stale'

    $dir = Join-Path $fixture 'never-configured'
    New-Item -ItemType Directory -Path $dir | Out-Null
    Check ($null -eq (Get-StaleToolchainReason $dir $installs)) 'a folder with no CMakeCache.txt is not stale'

    $dir = New-BuildDir 'compiler-gone' @($okInstance, $okLinker) (& $fwd (Join-Path $gone 'cl.exe'))
    $why = Get-StaleToolchainReason $dir $installs
    Check ($why -like '*compiler it saved*14.51.36231*') "a removed compiler is stale (got: $why)"

    $dir = New-BuildDir 'linker-gone' @($okInstance, "CMAKE_LINKER:FILEPATH=$(& $fwd (Join-Path $gone 'link.exe'))")
    $why = Get-StaleToolchainReason $dir $installs
    Check ($why -like 'CMAKE_LINKER*14.51.36231*') "a removed linker in the cache is stale (got: $why)"

    $dir = New-BuildDir 'instance-gone' @("CMAKE_GENERATOR_INSTANCE:INTERNAL=$(& $fwd (Join-Path $fixture 'VS\18\BuildTools'))")
    $why = Get-StaleToolchainReason $dir $installs
    Check ($why -like '*Visual Studio install it saved*BuildTools*') "a removed Visual Studio install is stale (got: $why)"

    $dir = New-BuildDir 'instance-old-version' @("CMAKE_GENERATOR_INSTANCE:INTERNAL=$(& $fwd $vs),version=18.9.12112.369", $okLinker)
    $why = Get-StaleToolchainReason $dir $installs
    Check ($why -like '*18.9.12112.369*now 18.12.11310.141*') "a saved Visual Studio version that was updated is stale (got: $why)"

    $dir = New-BuildDir 'instance-same-version' @("CMAKE_GENERATOR_INSTANCE:INTERNAL=$(& $fwd $vs),version=18.12.11310.141", $okLinker)
    Check ($null -eq (Get-StaleToolchainReason $dir $installs)) 'a saved Visual Studio version that is still installed is not stale'

    $dir = New-BuildDir 'relative-tool' @($okInstance, 'CMAKE_MT:FILEPATH=mt')
    Check ($null -eq (Get-StaleToolchainReason $dir $installs)) 'a tool saved by bare name, found on PATH, is not checked'
} finally {
    Remove-Item -LiteralPath $fixture -Recurse -Force -ErrorAction SilentlyContinue
}

if ($script:Failures.Count) {
    $script:Failures | ForEach-Object { Write-Host "FAIL: $_" }
    Write-Host "$($script:Passes) passed, $($script:Failures.Count) failed"
    exit 1
}
Write-Host "$($script:Passes) passed, 0 failed"
