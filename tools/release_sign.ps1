<#
.SYNOPSIS
Signs a Hydra Deluxe installer so the built-in updater will accept it.

.DESCRIPTION
Hashes the file with SHA-256, signs the hash with the release key, and writes
the 64-byte signature beside the file as <file>.sig. Hydra's verifier
(app::verify_update in src/app/update_verify.cpp) checks that signature
against the public key compiled into Hydra.

The key comes from -KeyPath, by default the file tools/release_keygen.ps1
wrote at %USERPROFILE%\.hydra-release\update-signing.xml. After signing, the
script checks the new signature with the key's public half and prints that
half, so you can see it matches src/app/update_key.h.

The steps for a release are in docs/development.md under "Releasing".

.EXAMPLE
pwsh -NoProfile -File tools/release_sign.ps1 -Installer build-installer\HydraDeluxe-2.11.0-setup.exe
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Installer,
    [string]$KeyPath,
    [string]$SignaturePath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if (-not $KeyPath) { $KeyPath = Join-Path $env:USERPROFILE '.hydra-release\update-signing.xml' }
$Installer = [System.IO.Path]::GetFullPath($Installer)
if (-not $SignaturePath) { $SignaturePath = "$Installer.sig" }
$SignaturePath = [System.IO.Path]::GetFullPath($SignaturePath)

if (-not (Test-Path -LiteralPath $Installer -PathType Leaf)) { throw "release_sign: no file at $Installer" }
if (-not (Test-Path -LiteralPath $KeyPath -PathType Leaf)) {
    throw "release_sign: no signing key at $KeyPath. Make one with tools/release_keygen.ps1, or pass -KeyPath."
}

$record = Import-Clixml -LiteralPath $KeyPath
if ($record.Kind -ne 'hydra-update-signing-key') { throw "release_sign: $KeyPath is not a Hydra update signing key." }
if ($record.Key -is [System.Security.SecureString]) {
    $bstr = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($record.Key)
    try { $secretText = [System.Runtime.InteropServices.Marshal]::PtrToStringBSTR($bstr) }
    finally { [System.Runtime.InteropServices.Marshal]::ZeroFreeBSTR($bstr) }
} else {
    $secretText = [string]$record.Key
}
$secret = [Convert]::FromBase64String($secretText)
$secretText = $null
if ($secret.Length -ne 96) { throw "release_sign: the key in $KeyPath has the wrong size." }

$params = New-Object System.Security.Cryptography.ECParameters
$params.Curve = [System.Security.Cryptography.ECCurve+NamedCurves]::nistP256
$params.D = [byte[]]$secret[0..31]
$point = New-Object System.Security.Cryptography.ECPoint
$point.X = [byte[]]$secret[32..63]
$point.Y = [byte[]]$secret[64..95]
$params.Q = $point
$public = [byte[]]$secret[32..95]

$sha = [System.Security.Cryptography.SHA256]::Create()
$stream = [System.IO.File]::OpenRead($Installer)
try { $hash = $sha.ComputeHash($stream) } finally { $stream.Dispose(); $sha.Dispose() }

$ecdsa = [System.Security.Cryptography.ECDsa]::Create($params)
try {
    # SignHash gives the IEEE P1363 form (r then s) that BCryptVerifySignature reads.
    $sig = $ecdsa.SignHash($hash)
    if ($sig.Length -ne 64) { throw "release_sign: .NET gave a $($sig.Length)-byte signature, not 64." }
    if (-not $ecdsa.VerifyHash($hash, $sig)) { throw 'release_sign: the new signature does not verify with its own key.' }
} finally {
    $ecdsa.Dispose()
}

[System.IO.File]::WriteAllBytes($SignaturePath, $sig)

Write-Output "Signed $Installer"
Write-Output "Signature written to $SignaturePath"
Write-Output ("SHA-256: " + (($hash | ForEach-Object { $_.ToString('x2') }) -join ''))
Write-Output ("Public key (compare with src/app/update_key.h): " + (($public | ForEach-Object { $_.ToString('x2') }) -join ''))
