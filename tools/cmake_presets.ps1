# A preset's build folder and configuration, read from CMakePresets.json
# (their one owner). Shared by build_cpp.ps1 and installer\build_installer.ps1.
# Dot-source it:
#   . (Join-Path <repo> "tools\cmake_presets.ps1")
#   $buildDir = Get-PresetBuildDir <repo> ship
#   $config   = Get-PresetBuildConfig <repo> ship

function Read-CMakePresets([string]$Repo) {
    return Get-Content (Join-Path $Repo "CMakePresets.json") -Raw | ConvertFrom-Json
}

function Get-PresetBuildDir([string]$Repo, [string]$Preset) {
    $presets = (Read-CMakePresets $Repo).configurePresets

    # binaryDir comes from the preset itself, else from what it inherits
    # (earlier parents first), as cmake resolves it.
    function Find-BinaryDir([string]$name) {
        $p = $presets | Where-Object { $_.name -eq $name }
        if (-not $p) { throw "no configure preset '$name' in CMakePresets.json" }
        if ($p.binaryDir) { return $p.binaryDir }
        foreach ($parent in @($p.inherits)) {
            if ($parent) {
                $dir = Find-BinaryDir $parent
                if ($dir) { return $dir }
            }
        }
        return $null
    }

    $dir = Find-BinaryDir $Preset
    if (-not $dir) { throw "preset '$Preset' has no binaryDir in CMakePresets.json" }
    $dir = $dir.Replace('${sourceDir}', $Repo).Replace('${presetName}', $Preset)
    # Any other macro (${...}, $env{...}, $penv{...}, $vendor{...}) is not expanded here.
    if ($dir -match '\$\w*\{') { throw "unsupported macro in binaryDir of preset '$Preset': $dir" }
    # cmake reads a relative binaryDir against the source folder.
    if (-not [IO.Path]::IsPathRooted($dir)) { $dir = Join-Path $Repo $dir }
    return [IO.Path]::GetFullPath($dir)
}

function Get-PresetBuildConfig([string]$Repo, [string]$Preset) {
    $p = (Read-CMakePresets $Repo).buildPresets | Where-Object { $_.name -eq $Preset }
    if (-not $p -or -not $p.configuration) {
        throw "no build preset '$Preset' with a configuration in CMakePresets.json"
    }
    return $p.configuration
}
