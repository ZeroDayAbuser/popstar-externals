# Writes src/sdk/build_seed_generated.hpp with a fresh 32-bit
# PAWJAWB_BUILD_SEED. Wired into vcxproj PreBuildEvent.

param(
    [string] $OutDir = (Join-Path $PSScriptRoot '..\src\sdk')
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Security
$rng     = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$buf     = New-Object byte[] 4
$rng.GetBytes($buf)
$seedHex = ([BitConverter]::ToUInt32($buf, 0)).ToString('X8')

$outFile = Join-Path $OutDir 'build_seed_generated.hpp'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$content = @"
#pragma once
#define PAWJAWB_BUILD_SEED 0x${seedHex}u
"@

$prev = $null
if (Test-Path $outFile) { $prev = Get-Content -Raw $outFile }
if ($prev -ne $content) {
    Set-Content -LiteralPath $outFile -Value $content -Encoding ASCII -NoNewline
    Write-Host "[build-seed] regenerated $outFile -> 0x$seedHex"
} else {
    Write-Host "[build-seed] seed unchanged (0x$seedHex)"
}
