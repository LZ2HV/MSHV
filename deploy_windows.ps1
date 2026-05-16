#Requires -Version 5.1
<#
.SYNOPSIS
  Build + package MSHV for Windows x86_64 (MinGW Qt5).

.DESCRIPTION
  Produces dist\MSHV-<version>-windows-x64.zip with MSHV.exe, the Qt5
  runtime DLLs deployed by windeployqt, the bin\settings tree, and a
  README. The .pro file already links FFTW + DirectSound statically, so
  the only DLLs in the bundle are Qt + MinGW runtime.

.PARAMETER Version
  Version string used in the zip filename. Defaults to "dev".

.NOTES
  Requires qmake, mingw32-make, and windeployqt on PATH. On the CI runner
  these come from jurplel/install-qt-action with arch=win64_mingw81 (and
  tools_mingw if mingw isn't on the runner's PATH already).
#>

[CmdletBinding()]
param(
    [string]$Version = "dev"
)

$ErrorActionPreference = "Stop"

$ProFile = "MSHV_WIN64.pro"
$BinName = "MSHV_WIN64.exe"
$DistName = "MSHV-$Version-windows-x64"
$DistDir = "dist\$DistName"

function Require-Tool {
    param([string]$Name)
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required tool '$Name' not found on PATH."
    }
}

Require-Tool qmake
Require-Tool windeployqt

$Make = if (Get-Command mingw32-make -ErrorAction SilentlyContinue) { "mingw32-make" } else { "make" }

Write-Host "==> qmake $ProFile"
& qmake $ProFile
if ($LASTEXITCODE -ne 0) { throw "qmake failed" }

Write-Host "==> $Make -j$env:NUMBER_OF_PROCESSORS"
& $Make "-j$env:NUMBER_OF_PROCESSORS"
if ($LASTEXITCODE -ne 0) { throw "make failed" }

if (-not (Test-Path "bin\$BinName")) {
    throw "bin\$BinName was not produced by the build."
}

Write-Host "==> Staging $DistDir"
if (Test-Path $DistDir) { Remove-Item -Recurse -Force $DistDir }
New-Item -ItemType Directory -Path $DistDir | Out-Null

Copy-Item "bin\$BinName" "$DistDir\MSHV.exe"

# Ship data dirs that live next to the binary. settings/ holds the templates
# the app copies on first run; skip caches and user-runtime dirs.
foreach ($d in @("settings", "help")) {
    if (Test-Path "bin\$d") {
        Copy-Item -Recurse "bin\$d" "$DistDir\"
    }
}
foreach ($f in @("README.txt", "COPYING.txt")) {
    if (Test-Path $f) { Copy-Item $f "$DistDir\" }
}

Write-Host "==> windeployqt MSHV.exe"
& windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$DistDir\MSHV.exe"
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }

@"
MSHV $Version - Windows x64

This bundle is self-contained: Qt5 + MinGW runtime DLLs were deployed
next to MSHV.exe by windeployqt. FFTW and DirectSound are statically
linked into the executable.

Run by double-clicking MSHV.exe.

If Windows SmartScreen blocks the first launch, click "More info" then
"Run anyway". MSHV is not signed by a registered Microsoft publisher.
"@ | Set-Content -Path "$DistDir\INSTALL.txt" -Encoding UTF8

if (-not (Test-Path dist)) { New-Item -ItemType Directory -Path dist | Out-Null }
$Zip = "dist\$DistName.zip"
if (Test-Path $Zip) { Remove-Item -Force $Zip }
Write-Host "==> Compressing $Zip"
Compress-Archive -Path $DistDir -DestinationPath $Zip -CompressionLevel Optimal

$SizeMB = [math]::Round((Get-Item $Zip).Length / 1MB, 1)
Write-Host ""
Write-Host "==> Release ready: $Zip ($SizeMB MB)"
