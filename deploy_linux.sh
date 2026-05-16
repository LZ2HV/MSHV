#!/bin/bash
set -euo pipefail

# Build + package MSHV for Linux x86_64.
# Produces dist/MSHV-<version>-linux-x86_64.tar.gz containing the binary,
# the bin/ data dirs (settings/help/etc.) and a launcher script.
#
# Usage: bash deploy_linux.sh [version]
#   version defaults to "dev" if not provided.

VERSION="${1:-dev}"
PRO_FILE="MSHV_x86_64.pro"
BIN_NAME="MSHV_x86_64"
DIST_NAME="MSHV-${VERSION}-linux-x86_64"
DIST_DIR="dist/${DIST_NAME}"

QMAKE="${QMAKE:-qmake}"
if ! command -v "${QMAKE}" >/dev/null 2>&1; then
    if command -v qmake-qt5 >/dev/null 2>&1; then
        QMAKE="qmake-qt5"
    else
        echo "Error: qmake (Qt5) not found. Install with: apt install qtbase5-dev qt5-qmake libqt5websockets5-dev"
        exit 1
    fi
fi

echo "==> Building with ${QMAKE} (${PRO_FILE})"
"${QMAKE}" "${PRO_FILE}"
make -j"$(nproc 2>/dev/null || echo 2)"

if [ ! -x "bin/${BIN_NAME}" ]; then
    echo "Error: bin/${BIN_NAME} not produced by build."
    exit 1
fi

echo "==> Staging ${DIST_DIR}"
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

cp "bin/${BIN_NAME}" "${DIST_DIR}/MSHV"
chmod +x "${DIST_DIR}/MSHV"

# Ship the data directories that ship in bin/ (settings templates, help text,
# etc.) but skip caches and user-generated runtime dirs.
for d in settings help; do
    if [ -d "bin/${d}" ]; then
        cp -R "bin/${d}" "${DIST_DIR}/"
    fi
done

cp -f README.txt "${DIST_DIR}/" 2>/dev/null || true
cp -f COPYING.txt "${DIST_DIR}/" 2>/dev/null || true

cat > "${DIST_DIR}/run-mshv.sh" << 'EOF'
#!/bin/bash
# Launcher that runs MSHV from the directory containing it, so the bundled
# settings/help dirs (which it expects next to the binary) are found.
cd "$(dirname "$(readlink -f "$0")")"
exec ./MSHV "$@"
EOF
chmod +x "${DIST_DIR}/run-mshv.sh"

cat > "${DIST_DIR}/INSTALL.txt" << EOF
MSHV ${VERSION} — Linux x86_64

Runtime dependencies (install via your distro package manager):
  Debian/Ubuntu:  sudo apt install libqt5widgets5 libqt5network5 libqt5websockets5 \\
                                   libfftw3-3 libasound2 libpulse0
  Fedora/RHEL:    sudo dnf install qt5-qtbase qt5-qtwebsockets fftw alsa-lib pulseaudio-libs

Run with:  ./run-mshv.sh
EOF

mkdir -p dist
TARBALL="dist/${DIST_NAME}.tar.gz"
tar -czf "${TARBALL}" -C dist "${DIST_NAME}"
SIZE=$(du -sh "${TARBALL}" | cut -f1)
echo ""
echo "==> Release ready: ${TARBALL} (${SIZE})"
