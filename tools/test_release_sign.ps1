<#
.SYNOPSIS
Checks that tools/release_sign.ps1 and Hydra's C++ verifier agree on the
signature format, and that tools/release_keygen.ps1 keeps the real key out of
the repo.

.DESCRIPTION
ECDSA signatures differ on every run, so a fresh signature can't be compared
byte for byte with the committed testdata/update/sample.bin.sig. Instead both
are checked here with .NET against the committed test public key, the same
key tests/test_update_verify.cpp checks the committed .sig with in C++.

Prints one line per check and ends with PASS or FAIL and the counts; the exit
code is 0 only when every check passed.

.EXAMPLE
pwsh -NoProfile -File tools/test_release_sign.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$updateDir = Join-Path $repoRoot 'testdata\update'
$sample = Join-Path $updateDir 'sample.bin'
$keygen = Join-Path $PSScriptRoot 'release_keygen.ps1'
$sign = Join-Path $PSScriptRoot 'release_sign.ps1'
$shell = [System.Diagnostics.Process]::GetCurrentProcess().MainModule.FileName

$scratch = Join-Path ([System.IO.Path]::GetTempPath()) "hydra_test_release_sign_$PID"
if (Test-Path -LiteralPath $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force }
New-Item -ItemType Directory -Path $scratch | Out-Null

$inRepo = $null
$script:passed = 0
$script:failed = 0
function Check([string]$name, [bool]$ok) {
    if ($ok) { $script:passed++; Write-Output "ok   $name" }
    else { $script:failed++; Write-Output "FAIL $name" }
}

# Runs one of the two scripts in a child shell; returns its exit code and output.
function Invoke-Tool([string]$path, [string[]]$arguments) {
    # Windows PowerShell turns a child's stderr into a terminating error under
    # 'Stop'; the refusals checked below write there on purpose.
    $ErrorActionPreference = 'Continue'
    $out = & $shell -NoProfile -File $path @arguments 2>&1 | Out-String
    return [pscustomobject]@{ Code = $LASTEXITCODE; Output = $out }
}

function Read-PublicKey([string]$path) {
    $hex = ([System.IO.File]::ReadAllText($path)).Trim()
    if ($hex.Length -ne 128) { throw "$path does not hold 128 hex digits" }
    return [byte[]](0..63 | ForEach-Object { [Convert]::ToByte($hex.Substring(2 * $_, 2), 16) })
}

# Whether `sig` is a valid signature of the file's SHA-256 under `publicKey`
# (X then Y), checked by .NET rather than by the script that signed.
function Test-Signature([string]$file, [byte[]]$sig, [byte[]]$publicKey) {
    $params = New-Object System.Security.Cryptography.ECParameters
    $params.Curve = [System.Security.Cryptography.ECCurve+NamedCurves]::nistP256
    $point = New-Object System.Security.Cryptography.ECPoint
    $point.X = [byte[]]$publicKey[0..31]
    $point.Y = [byte[]]$publicKey[32..63]
    $params.Q = $point
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try { $hash = $sha.ComputeHash([System.IO.File]::ReadAllBytes($file)) } finally { $sha.Dispose() }
    $ecdsa = [System.Security.Cryptography.ECDsa]::Create($params)
    try { return $ecdsa.VerifyHash($hash, $sig) } finally { $ecdsa.Dispose() }
}

try {
    $testPublic = Read-PublicKey (Join-Path $updateDir 'test-public-key.txt')

    # A fresh signature from release_sign.ps1 with the committed test key.
    $freshSig = Join-Path $scratch 'sample.bin.sig'
    $r = Invoke-Tool $sign @('-Installer', $sample, '-KeyPath', (Join-Path $updateDir 'test-key.xml'), '-SignaturePath', $freshSig)
    Check 'release_sign.ps1 signs sample.bin with the test key' ($r.Code -eq 0 -and (Test-Path -LiteralPath $freshSig))
    $fresh = [System.IO.File]::ReadAllBytes($freshSig)
    Check 'the fresh signature is 64 bytes' ($fresh.Length -eq 64)
    Check 'the fresh signature verifies with the test public key' (Test-Signature $sample $fresh $testPublic)

    # The committed signature, the one the C++ test checks.
    $committed = [System.IO.File]::ReadAllBytes((Join-Path $updateDir 'sample.bin.sig'))
    Check 'the committed sample.bin.sig is 64 bytes' ($committed.Length -eq 64)
    Check 'the committed sample.bin.sig verifies with the test public key' (Test-Signature $sample $committed $testPublic)
    $flipped = [byte[]]$committed.Clone()
    $flipped[0] = $flipped[0] -bxor 1
    Check 'a changed signature does not verify' (-not (Test-Signature $sample $flipped $testPublic))

    # A real (DPAPI-protected) key outside the repo: made, used, checked.
    $realKey = Join-Path $scratch 'update-signing.xml'
    $realPublic = Join-Path $scratch 'public.txt'
    $r = Invoke-Tool $keygen @('-KeyPath', $realKey, '-PublicKeyOut', $realPublic)
    Check 'release_keygen.ps1 makes a protected key outside the repo' ($r.Code -eq 0 -and (Test-Path -LiteralPath $realKey))
    Check 'the protected key file holds no plain key text' (-not ((Get-Content -LiteralPath $realKey -Raw) -match '<S N="Key">'))
    $realSig = Join-Path $scratch 'real.sig'
    $r = Invoke-Tool $sign @('-Installer', $sample, '-KeyPath', $realKey, '-SignaturePath', $realSig)
    Check 'release_sign.ps1 signs with the protected key' ($r.Code -eq 0)
    Check 'that signature verifies with the printed public key' (Test-Signature $sample ([System.IO.File]::ReadAllBytes($realSig)) (Read-PublicKey $realPublic))
    Check 'that signature does not verify with the test key' (-not (Test-Signature $sample ([System.IO.File]::ReadAllBytes($realSig)) $testPublic))

    # The keygen's refusals.
    $r = Invoke-Tool $keygen @('-KeyPath', $realKey)
    Check 'release_keygen.ps1 refuses to overwrite a key file' ($r.Code -ne 0)
    $inRepo = Join-Path $repoRoot "testdata\update\should-not-exist-$PID.xml"
    $r = Invoke-Tool $keygen @('-KeyPath', $inRepo)
    Check 'release_keygen.ps1 refuses a protected key inside the repo' ($r.Code -ne 0 -and -not (Test-Path -LiteralPath $inRepo))
    $r = Invoke-Tool $keygen @('-TestKey')
    Check 'release_keygen.ps1 -TestKey needs an explicit -KeyPath' ($r.Code -ne 0)
} finally {
    Remove-Item -LiteralPath $scratch -Recurse -Force -ErrorAction SilentlyContinue
    # Only there if the in-repo refusal above failed.
    if ($inRepo -and (Test-Path -LiteralPath $inRepo)) { Remove-Item -LiteralPath $inRepo -Force }
}

if ($script:failed -eq 0) {
    Write-Output "PASS $($script:passed) checks"
    exit 0
}
Write-Output "FAIL $($script:failed) of $($script:passed + $script:failed) checks"
exit 1
