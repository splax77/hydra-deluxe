# A preset's build folder, read from CMakePresets.json (its one owner).
# Shared by build_cpp.ps1 and installer\build_installer.ps1. Dot-source it:
#   . (Join-Path <repo> "tools\cmake_presets.ps1")
#   $buildDir = Get-PresetBuildDir <repo> ship

function Get-PresetBuildDir([string]$Repo, [string]$Preset) {
    $presets = (Get-Content (Join-Path $Repo "CMakePresets.json") -Raw | ConvertFrom-Json).configurePresets

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
    if ($dir -match '\$\{') { throw "unsupported macro in binaryDir of preset '$Preset': $dir" }
    return [IO.Path]::GetFullPath($dir)
}
