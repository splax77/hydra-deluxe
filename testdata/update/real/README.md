# Real files from GitHub for the updater tests

The five files in this folder are what GitHub sent for Hydra Deluxe's releases on 2026-11-21. Each one is the unedited output of the command listed below. Nobody typed or changed a byte of them. The `.gitattributes` beside them marks every file binary, so git never converts their line endings.

Refresh them only by re-running these same commands, then re-run the checks below and replace their output here. Never edit a file by hand.

The installer was downloaded only. It was never run, opened or unpacked.

## Where each file came from

`latest.json` is GitHub's reply for the latest release. The user saved it on 2026-11-21 with the command below, and GitHub answered with status 200.

    cmd /c "gh api repos/splax77/hydra-deluxe/releases/latest > testdata\update\real\latest.json"

`not-found.json` is GitHub's reply for a release tag that does not exist. The user saved it on 2026-11-21 with the command below, and GitHub answered with status 404.

    cmd /c "gh api repos/splax77/hydra-deluxe/releases/tags/v0.0.0-does-not-exist > testdata\update\real\not-found.json"

`HydraDeluxe-3.1.2-setup.exe` and `HydraDeluxe-3.1.2-setup.exe.sig` are the installer and its signature from release v3.1.2, the release `latest.json` names. The main session saved both on 2026-11-21 with the command below. `gh release download` does not print GitHub's status code; it writes a file only when GitHub answers with success (status 200), and stops with an error on any other status. Both files match the size and digest the reply states (see the checks below).

    gh release download v3.1.2 --repo splax77/hydra-deluxe --dir testdata/update/real

`other-release.sig` is the signature from release v3.1.1. It is a real signature of a different installer, so it never matches the v3.1.2 installer. The main session saved it on 2026-11-21 with the command below, from the `testdata/update/real` folder. As above, the command writes the file only on a success status (200).

    gh release download v3.1.1 --repo splax77/hydra-deluxe --pattern "*.sig" --output other-release.sig

## Checks, and their output on 2026-11-21

These checks need no build. They were run in PowerShell from this folder.

**The installer matches what the reply states.** The reply gives the installer's size and SHA-256 digest. The file on disk has the same of both.

    $j = Get-Content -Raw latest.json | ConvertFrom-Json
    $j.assets | ForEach-Object { "asset: $($_.name) size=$($_.size) digest=$($_.digest)" }
    Get-FileHash -Algorithm SHA256 HydraDeluxe-3.1.2-setup.exe
    (Get-Item HydraDeluxe-3.1.2-setup.exe).Length

    asset: HydraDeluxe-3.1.2-setup.exe size=33990889 digest=sha256:4a510fc7d4e539998fc41b53161eda4d0b19cb125bb58517a6440460670dcd5c
    asset: HydraDeluxe-3.1.2-setup.exe.sig size=64 digest=sha256:0e979bef35af86e2a185bdf73c003b815a18089787a8e515901f9e3d0c9d31e8
    Hash : 4A510FC7D4E539998FC41B53161EDA4D0B19CB125BB58517A6440460670DCD5C
    33990889

**Both signatures are 64 bytes, and they differ.** The v3.1.2 signature's hash also equals the digest the reply states for it.

    (Get-Item HydraDeluxe-3.1.2-setup.exe.sig).Length
    (Get-Item other-release.sig).Length
    Get-FileHash -Algorithm SHA256 HydraDeluxe-3.1.2-setup.exe.sig, other-release.sig

    64
    64
    0E979BEF35AF86E2A185BDF73C003B815A18089787A8E515901F9E3D0C9D31E8 HydraDeluxe-3.1.2-setup.exe.sig
    A23B5F2C302381933F776927185C51200096C5B91A62EA82AC2E238E74ED8BBD other-release.sig

**The reply has exactly one installer and one signature.** It holds two assets in all.

    $j.assets.Count
    @($j.assets | Where-Object { $_.name -like '*-setup.exe' }).Count
    @($j.assets | Where-Object { $_.name -like '*-setup.exe.sig' }).Count

    2
    1
    1

**The reply's version is not newer than Hydra's own.** The reply names v3.1.2, and `CMakeLists.txt` line 21 sets the project version to 3.2.0, so the reply is older.

    $j.tag_name
    Select-String -Path ..\..\..\CMakeLists.txt -Pattern 'project\s*\('

    v3.1.2
    CMakeLists.txt:21:project(Hydra VERSION 3.2.0 LANGUAGES C CXX RC)

**The two replies are plain command output.** Each starts with `{` and ends with `}`, with no byte-order mark and no line breaks. The 404 reply reads in full:

    {"message":"Not Found","documentation_url":"https://docs.github.com/rest/releases/releases#get-a-release-by-tag-name","status":"404"}
