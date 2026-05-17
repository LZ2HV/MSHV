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

# Resolve the Qt install root (set by install-qt-action on CI, or fall back
# to the directory containing qmake locally) so we invoke the windeployqt
# that matches the Qt we built against — using whatever's on PATH can pick
# up a stale Qt and breaks platform plugin lookup.
$QtRoot = $env:QT_ROOT_DIR
if (-not $QtRoot) {
    $qmakePath = (Get-Command qmake).Source
    $QtRoot = Split-Path -Parent (Split-Path -Parent $qmakePath)
}
$WindeployQt = Join-Path $QtRoot "bin\windeployqt.exe"
if (-not (Test-Path $WindeployQt)) { throw "windeployqt.exe not found at $WindeployQt" }

Write-Host "==> Qt root: $QtRoot"
Write-Host "==> windeployqt: $WindeployQt"
$PluginsDir = Join-Path $QtRoot "plugins"
$PlatformsDir = Join-Path $PluginsDir "platforms"
Write-Host "==> Contents of $PlatformsDir :"
if (Test-Path $PlatformsDir) {
    Get-ChildItem $PlatformsDir | ForEach-Object { Write-Host "    $($_.Name)" }
} else {
    Write-Host "    (missing!)"
}

Write-Host "==> windeployqt MSHV.exe"
& $WindeployQt --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$DistDir\MSHV.exe"
$wdqExit = $LASTEXITCODE

# windeployqt aborts on "Unable to find the platform plugin" without copying
# anything — not even the Qt5*.dll runtimes it just announced as "To be
# deployed". Detect that and do a full manual copy from $QtRoot. The module
# list mirrors `QT += widgets axcontainer network websockets` in
# MSHV_WIN64.pro plus the MinGW C++ runtime DLLs Qt was built against.
if (-not (Test-Path (Join-Path $DistDir "Qt5Core.dll"))) {
    Write-Host "==> windeployqt did not deploy runtime DLLs — copying manually"
    $QtBin = Join-Path $QtRoot "bin"

    $QtDlls = @(
        "Qt5Core.dll", "Qt5Gui.dll", "Qt5Widgets.dll",
        "Qt5Network.dll", "Qt5WebSockets.dll",
        "Qt5AxContainer.dll", "Qt5AxBase.dll"
    )
    foreach ($dll in $QtDlls) {
        $src = Join-Path $QtBin $dll
        if (Test-Path $src) {
            Copy-Item $src $DistDir
        } else {
            Write-Host "    (skipping $dll — not present in Qt install)"
        }
    }

    # MinGW C++ runtime — these live next to qmake.exe.
    $RuntimeDlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")
    foreach ($dll in $RuntimeDlls) {
        $src = Join-Path $QtBin $dll
        if (Test-Path $src) { Copy-Item $src $DistDir }
        else { throw "Required MinGW runtime $dll not found at $src" }
    }

    # Plugins. platforms/qwindows.dll is mandatory — without it the app
    # refuses to start. styles + imageformats are nice-to-haves for native
    # look-and-feel and PNG/JPEG/ICO support; copy if present.
    $DistPlatforms = Join-Path $DistDir "platforms"
    New-Item -ItemType Directory -Force -Path $DistPlatforms | Out-Null
    $qwindows = Join-Path $PlatformsDir "qwindows.dll"
    if (-not (Test-Path $qwindows)) { throw "qwindows.dll not found at $qwindows" }
    Copy-Item $qwindows $DistPlatforms

    foreach ($subdir in @("styles", "imageformats")) {
        $srcDir = Join-Path $PluginsDir $subdir
        if (Test-Path $srcDir) {
            $dstDir = Join-Path $DistDir $subdir
            New-Item -ItemType Directory -Force -Path $dstDir | Out-Null
            Copy-Item (Join-Path $srcDir "*.dll") $dstDir
        }
    }

    # qt.conf — tells the binary plugins live next to the exe rather than
    # in some baked-in absolute path. Belt-and-braces; the default search
    # would find them anyway in this flat layout.
    "[Paths]`nPlugins = ." | Set-Content -Path (Join-Path $DistDir "qt.conf") -Encoding ASCII
}

if (-not (Test-Path (Join-Path $DistDir "Qt5Core.dll"))) {
    throw "Qt5Core.dll missing from bundle after recovery — aborting"
}
# windeployqt's nonzero exit lingers in $LASTEXITCODE even after we recover;
# the GitHub Actions pwsh shell uses it as the step's final exit code unless
# we clear it. Reset to 0 now that we know the bundle is valid.
$global:LASTEXITCODE = 0

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
exit 0
