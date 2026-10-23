<#
.SYNOPSIS
Makes the key pair that signs Hydra Deluxe's update installers. Run it once.

.DESCRIPTION
Creates a new ECDSA key on the P-256 curve with .NET and prints its public
half as the C++ array to paste into src/app/update_key.h. The private half is
written to -KeyPath, by default %USERPROFILE%\.hydra-release\update-signing.xml.

The private key file is protected with Windows DPAPI (Export-Clixml of a
SecureString), so only the same Windows user on the same PC can read it.
Back it up in your password store. A lost key means a new key, and one last
release that players install by hand.

The script refuses to write a protected key anywhere inside this repo, and it
never overwrites an existing key file.

-TestKey writes an unprotected key instead, for the committed test pair in
testdata/update/ (that key is not secret). It needs an explicit -KeyPath.

The signing side is tools/release_sign.ps1; the steps for a release are in
docs/development.md under "Releasing".

.EXAMPLE
pwsh -NoProfile -File tools/release_keygen.ps1

.EXAMPLE
pwsh -NoProfile -File tools/release_keygen.ps1 -TestKey -KeyPath testdata/update/test-key.xml -PublicKeyOut testdata/update/test-public-key.txt
#>
[CmdletBinding()]
param(
    [string]$KeyPath,
    [string]$PublicKeyOut,
    [switch]$TestKey
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

if (-not $KeyPath) {
    if ($TestKey) { throw 'release_keygen: -TestKey needs an explicit -KeyPath, so a test key never lands at the real key''s path.' }
    $KeyPath = Join-Path $env:USERPROFILE '.hydra-release\update-signing.xml'
}
$KeyPath = [System.IO.Path]::GetFullPath($KeyPath)

# Whether `path` is the repo folder or anything under it.
function Test-UnderRepo([string]$path) {
    $root = $repoRoot.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    return ($path.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar).StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)
}

if (-not $TestKey -and (Test-UnderRepo $KeyPath)) {
    throw "release_keygen: refusing to write the real signing key inside the repo ($KeyPath). Leave -KeyPath out to use the default under %USERPROFILE%\.hydra-release."
}
if (Test-Path -LiteralPath $KeyPath) {
    throw "release_keygen: $KeyPath already exists. Delete it yourself first if you really mean to replace that key."
}

$ecdsa = [System.Security.Cryptography.ECDsa]::Create([System.Security.Cryptography.ECCurve+NamedCurves]::nistP256)
try {
    $p = $ecdsa.ExportParameters($true)
    if ($p.D.Length -ne 32 -or $p.Q.X.Length -ne 32 -or $p.Q.Y.Length -ne 32) {
        throw 'release_keygen: .NET gave a P-256 key whose parts are not 32 bytes each.'
    }
    $public = [byte[]]($p.Q.X + $p.Q.Y)
    # The key file holds D, X and Y as one base64 string; release_sign.ps1 reads it back.
    $secretText = [Convert]::ToBase64String([byte[]]($p.D + $public))
} finally {
    $ecdsa.Dispose()
}

$dir = Split-Path -Parent $KeyPath
if (-not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Path $dir | Out-Null }

if ($TestKey) {
    $record = [pscustomobject]@{ Kind = 'hydra-update-signing-key'; Protected = $false; Key = $secretText }
} else {
    $secure = ConvertTo-SecureString -String $secretText -AsPlainText -Force
    $record = [pscustomobject]@{ Kind = 'hydra-update-signing-key'; Protected = $true; Key = $secure }
}
$record | Export-Clixml -LiteralPath $KeyPath
$secretText = $null

$hex = ($public | ForEach-Object { $_.ToString('x2') }) -join ''
if ($PublicKeyOut) {
    [System.IO.File]::WriteAllText([System.IO.Path]::GetFullPath($PublicKeyOut), $hex + "`n")
}

$lines = for ($i = 0; $i -lt 64; $i += 8) {
    '    ' + (($public[$i..($i + 7)] | ForEach-Object { '0x' + $_.ToString('x2') }) -join ', ') + ','
}
Write-Output "Private key written to $KeyPath"
if (-not $TestKey) { Write-Output 'Back this file up in your password store now.' }
Write-Output ''
Write-Output 'Public key, for kUpdatePublicKey in src/app/update_key.h:'
Write-Output ($lines -join "`n")
Write-Output ''
Write-Output "Public key as hex: $hex"
