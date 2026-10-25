# Configure and build the Hydra C++ port.
#
#   .\build_cpp.ps1              # configure (if needed) + build Release, with
#                                # hydra_bench and hydra_replay
#   .\build_cpp.ps1 -Configure   # force a reconfigure first
#   .\build_cpp.ps1 -Target hydra_tests
#   .\build_cpp.ps1 -Target hydra_tests,hydra_uitest  # several targets, one build
#   .\build_cpp.ps1 -Preset ship # the installer's build, in build-ship\
#   .\build_cpp.ps1 -Preset asan -Target hydra_tests  # AddressSanitizer, in build-asan\
#   .\build_cpp.ps1 -Package     # build, then zip a release (CPack)
#
# CMake and MSVC ship with Visual Studio, so this finds the VS-bundled cmake.exe
# via vswhere rather than requiring cmake on PATH.

param(
    [switch]$Configure,
    [switch]$Package,
    # One or more targets. "pwsh -File" hands "a,b" over as one string, so
    # commas inside a name split it too.
    [string[]]$Target = @(),
    # default: the build preset's configuration in CMakePresets.json (Release).
    [string]$Config = "",
    # default: dev build in build-cpp. vs2022: the VS 2022 generator, in its
    # own folder. ship: the installer's build (no attached GUI tests).
    # asan: the AddressSanitizer test build, which never ships. coverage and
    # fuzz: the coverage and fuzzing builds, which never ship either.
    # asan-ninja: the AddressSanitizer build through Ninja and sccache, a CI
    # trial (CMakePresets.json says what it sets).
    [ValidateSet("default", "vs2022", "ship", "asan", "asan-ninja", "coverage", "fuzz")]
    [string]$Preset = "default"
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

. (Join-Path $root "tools\find_cmake.ps1")
. (Join-Path $root "tools\cmake_presets.ps1")
$cmake = Find-CMake
Write-Host "Using cmake: $cmake"

$buildDir = Get-PresetBuildDir $root $Preset
if ($Config -eq "") { $Config = Get-PresetBuildConfig $root $Preset }

# A Ninja preset has no Visual Studio solution to find the compiler through,
# so it needs cl.exe and ninja.exe on PATH, as a Visual Studio developer shell
# gives. CI's ilammy/msvc-dev-cmd step already gives one; otherwise enter it
# here, for this PowerShell session.
$generator = ((Read-CMakePresets $root).configurePresets | Where-Object { $_.name -eq $Preset }).generator
$ninja = "$generator" -like "Ninja*"
if ($ninja -and -not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    # The newest install, as the Visual Studio generator picks; -all because
    # -latest skips an install with a pending update. An older compiler beside
    # it (VS 2022's 14.44) fails on raw strings inside the test macros.
    $vs = (& $vswhere -all -prerelease -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json |
        ConvertFrom-Json | Sort-Object { [version]$_.installationVersion } -Descending |
        Select-Object -First 1).installationPath
    if (-not $vs) { throw "no Visual Studio C++ install found for the Ninja preset '$Preset'" }
    Import-Module (Join-Path $vs "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
    Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64" | Out-Null
}

# cmake reads CMakePresets.json from the current folder, so run it from the
# script's own checkout; otherwise a call from another worktree builds that one.
Push-Location $root
try {
    # The installer's build always configures, so it always takes its preset's
    # settings (link-time optimization on, GUI tests out), even in a build
    # folder made before a preset changed. It is built rarely; this costs seconds.
    if ($Configure -or $Preset -eq "ship" -or -not (Test-Path (Join-Path $buildDir "CMakeCache.txt"))) {
        & $cmake --preset $Preset
        if ($LASTEXITCODE -ne 0) { throw "configure failed" }
    }

    # The test-file glob in CMakeLists.txt is re-checked only inside the build,
    # by ZERO_CHECK. By then MSBuild has already loaded the old project files,
    # so a new test file would be left out of this build and picked up only by
    # the next one. Running ZERO_CHECK on its own first regenerates the
    # projects before the real build loads them. Ninja has no ZERO_CHECK: it
    # re-checks the glob itself before every build.
    if (-not $ninja) {
        & $cmake --build --preset $Preset --config $Config --target ZERO_CHECK
        if ($LASTEXITCODE -ne 0) { throw "ZERO_CHECK (the regeneration check) failed" }
    }

    # --parallel with no count lets MSBuild build independent projects at
    # once, uncapped (decision D86.5).
    $buildArgs = @("--build", "--preset", $Preset, "--config", $Config, "--parallel")
    $targets = @($Target | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne "" })
    if ($targets.Count -gt 0) {
        foreach ($t in $targets) { $buildArgs += @("--target", $t) }
    } else {
        # hydra_bench and hydra_replay are EXCLUDE_FROM_ALL (not shipped), so a
        # plain build names them too; otherwise they could break unnoticed.
        # Ninja calls the everything target "all", Visual Studio "ALL_BUILD".
        $all = if ($ninja) { "all" } else { "ALL_BUILD" }
        $buildArgs += @("--target", $all, "hydra_bench", "hydra_replay")
    }
    & $cmake @buildArgs
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    Pop-Location
}

Write-Host "Build succeeded. Artifacts in $buildDir\$Config\"

if ($Package) {
    # cpack.exe sits next to cmake.exe in the VS bundle.
    $cpack = Join-Path (Split-Path $cmake) "cpack.exe"
    Push-Location $buildDir
    try {
        & $cpack -G ZIP -C $Config
        if ($LASTEXITCODE -ne 0) { throw "cpack failed" }
    } finally {
        Pop-Location
    }
    Write-Host "Package written to $buildDir\package\"
}
