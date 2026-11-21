# Whether a build folder's saved toolchain still exists on this machine.
#
#   . tools\stale_toolchain.ps1
#   $why = Get-StaleToolchainReason <build folder>   # $null when nothing is stale
#
# CMake saves the tools it found in the build folder: the Visual Studio install
# (CMAKE_GENERATOR_INSTANCE, sometimes with its exact version), the linker and
# librarian in CMakeCache.txt, and the compiler in
# CMakeFiles\<cmake version>\CMake<lang>Compiler.cmake. A Visual Studio update
# can remove the toolset those paths name, or move the install to a new
# version. CMake then refuses to configure that folder ("is not a full path to
# an existing compiler tool", "could not find specified instance of Visual
# Studio"), and running it again does not always clear the record.
# build_cpp.ps1 asks this file, and configures with --fresh when it answers.

# The cache entries that hold a tool path CMake found by itself.
$StaleToolchainCacheKeys = @('CMAKE_C_COMPILER', 'CMAKE_CXX_COMPILER', 'CMAKE_LINKER', 'CMAKE_AR', 'CMAKE_MT')

# The installed Visual Studio versions by install folder, from vswhere, or an
# empty table when vswhere is missing.
function Get-VisualStudioVersions {
    $table = @{}
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { return $table }
    $installs = & $vswhere -all -prerelease -products * -format json | ConvertFrom-Json
    foreach ($i in $installs) {
        $table[([IO.Path]::GetFullPath($i.installationPath)).TrimEnd('\').ToLowerInvariant()] = "$($i.installationVersion)"
    }
    $table
}

# The first reason the folder's saved toolchain is gone, in words, or $null.
# A folder with no CMakeCache.txt has nothing saved, so it is never stale.
# -Installs replaces the vswhere table, for the self-test.
function Get-StaleToolchainReason([string]$BuildDir, [hashtable]$Installs = $null) {
    $cache = Join-Path $BuildDir 'CMakeCache.txt'
    if (-not (Test-Path -LiteralPath $cache)) { return $null }
    $lines = Get-Content -LiteralPath $cache

    foreach ($line in $lines) {
        if ($line -notmatch '^CMAKE_GENERATOR_INSTANCE:[A-Z]+=(.+)$') { continue }
        $parts = $Matches[1].Trim() -split ','
        $path = $parts[0]
        if (-not (Test-Path -LiteralPath $path)) { return "the Visual Studio install it saved, $path, is gone" }
        $wanted = ($parts | Where-Object { $_ -like 'version=*' } | Select-Object -First 1)
        if ($wanted) {
            if ($null -eq $Installs) { $Installs = Get-VisualStudioVersions }
            $key = ([IO.Path]::GetFullPath($path)).TrimEnd('\').ToLowerInvariant()
            $have = $Installs[$key]
            $want = $wanted.Substring('version='.Length)
            if ($have -and $have -ne $want) { return "it saved Visual Studio $want at $path, which is now $have" }
        }
    }

    foreach ($line in $lines) {
        if ($line -notmatch '^([A-Z_]+):FILEPATH=(.+)$') { continue }
        if ($Matches[1] -notin $StaleToolchainCacheKeys) { continue }
        $tool = $Matches[2].Trim()
        if ([IO.Path]::IsPathRooted($tool) -and -not (Test-Path -LiteralPath $tool)) {
            return "$($Matches[1]), $tool, is gone"
        }
    }

    $files = Get-ChildItem -Path (Join-Path $BuildDir 'CMakeFiles') -Filter 'CMake*Compiler.cmake' -Recurse -Depth 1 -ErrorAction SilentlyContinue
    foreach ($f in $files) {
        foreach ($line in (Get-Content -LiteralPath $f.FullName)) {
            if ($line -notmatch '^set\((CMAKE_(C|CXX)_COMPILER) "(.+)"\)$') { continue }
            $tool = $Matches[3]
            if ([IO.Path]::IsPathRooted($tool) -and -not (Test-Path -LiteralPath $tool)) {
                return "the compiler it saved, $tool, is gone"
            }
        }
    }
    $null
}
