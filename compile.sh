#!/bin/bash
set -euo pipefail

QT5_PREFIX=$(brew --prefix qt@5 2>/dev/null || true)
if [ -z "${QT5_PREFIX}" ]; then
    echo "Error: qt@5 not found. Install with: brew install qt@5"
    exit 1
fi

QMAKE="${QT5_PREFIX}/bin/qmake"
if [ ! -x "${QMAKE}" ]; then
    echo "Error: qmake not found at ${QMAKE}"
    exit 1
fi

echo "Using qmake: ${QMAKE}"
"${QMAKE}" MSHV_macOS.pro
make -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc)"
