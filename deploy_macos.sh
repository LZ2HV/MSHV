#!/bin/bash
set -euo pipefail

APP="bin/MSHV.app"
BINARY="${APP}/Contents/MacOS/MSHV"
FW="${APP}/Contents/Frameworks"

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

QT_INSTALL_LIBS=$("${QMAKE}" -query QT_INSTALL_LIBS)
QT_INSTALL_PLUGINS=$("${QMAKE}" -query QT_INSTALL_PLUGINS)

if [ ! -f "${BINARY}" ]; then
    echo "Error: ${BINARY} not found. Build first with: bash compile.sh"
    exit 1
fi

echo "==> Detecting paths..."
echo "    qmake:        ${QMAKE}"
echo "    Qt libs:      ${QT_INSTALL_LIBS}"
echo "    Qt plugins:   ${QT_INSTALL_PLUGINS}"

echo "==> Step 0: Cleaning previously deployed Frameworks/PlugIns..."
rm -rf "${APP}/Contents/Frameworks" "${APP}/Contents/PlugIns"
codesign --remove-signature "${BINARY}" 2>/dev/null || true

if otool -L "${BINARY}" | grep -qE '/(opt|usr/local)/homebrew|/Volumes/FastDisk/homebrew'; then
    echo "==> Step 1: Running macdeployqt..."
    macdeployqt "${APP}" -verbose=1 -no-plugins
else
    echo "==> Step 1: Binary already has @executable_path refs — skipping macdeployqt."
    echo "    (Manual deployment in Steps 3-5 will copy everything needed.)"
fi

echo "==> Step 2: Deploying cocoa platform plugin..."
mkdir -p "${APP}/Contents/PlugIns/platforms"
cp -f "${QT_INSTALL_PLUGINS}/platforms/libqcocoa.dylib" "${APP}/Contents/PlugIns/platforms/"
mkdir -p "${APP}/Contents/Resources"
cat > "${APP}/Contents/Resources/qt.conf" << 'EOF'
[Paths]
Plugins = PlugIns
EOF

echo "==> Step 3: Copying required dylibs into Frameworks..."
mkdir -p "${FW}"

brew_cp() {
    local pkg="$1"
    local libfile="$2"
    local prefix
    prefix=$(brew --prefix "${pkg}" 2>/dev/null) || true
    if [ -n "${prefix}" ] && [ -f "${prefix}/lib/${libfile}" ]; then
        cp -f "${prefix}/lib/${libfile}" "${FW}/"
    fi
}

brew_cp fftw libfftw3.3.dylib
brew_cp portaudio libportaudio.2.dylib
brew_cp freetype libfreetype.6.dylib
brew_cp libpng libpng16.16.dylib
brew_cp pcre2 libpcre2-16.0.dylib
brew_cp pcre2 libpcre2-8.0.dylib
brew_cp zstd libzstd.1.dylib
brew_cp glib libglib-2.0.0.dylib
brew_cp glib libgthread-2.0.0.dylib
brew_cp gettext libintl.8.dylib
brew_cp md4c libmd4c.0.dylib

deploy_qt_framework() {
    local name="$1"
    local src_fw="${QT_INSTALL_LIBS}/${name}.framework"
    local src="${src_fw}/Versions/5/${name}"
    local dst_fw="${FW}/${name}.framework"
    local dst="${dst_fw}/Versions/5"
    if [ -f "${src}" ] && [ ! -f "${dst}/${name}" ]; then
        mkdir -p "${dst}"
        cp -f "${src}" "${dst}/"
        if [ -d "${src_fw}/Versions/5/Resources" ]; then
            cp -R "${src_fw}/Versions/5/Resources" "${dst}/"
        fi
        ln -sf 5 "${dst_fw}/Versions/Current"
        ln -sf Versions/Current/"${name}" "${dst_fw}/${name}"
        ln -sf Versions/Current/Resources "${dst_fw}/Resources"
    fi
}

for fw_name in QtCore QtGui QtWidgets QtNetwork QtWebSockets \
               QtDBus QtPrintSupport QtOpenGL; do
    deploy_qt_framework "${fw_name}"
done

echo "==> Step 4: Fixing all install names..."

collect_files() {
    find "${FW}" "${APP}/Contents/PlugIns" -type f \( -name '*.dylib' -o \
        -name 'QtCore' -o -name 'QtGui' -o -name 'QtWidgets' -o \
        -name 'QtNetwork' -o -name 'QtWebSockets' -o -name 'QtOpenGL' -o \
        -name 'QtDBus' -o -name 'QtPrintSupport' \) 2>/dev/null || true
}

set_id() {
    local target="$1"
    local fname
    fname=$(basename "${target}")
    if echo "${target}" | grep -q '\.framework/'; then
        local fw_name
        fw_name=$(echo "${target}" | sed 's|.*/\([^/]*\)\.framework.*|\1|')
        install_name_tool -id "@executable_path/../Frameworks/${fw_name}.framework/Versions/5/${fw_name}" "${target}" 2>/dev/null || true
    else
        install_name_tool -id "@executable_path/../Frameworks/${fname}" "${target}" 2>/dev/null || true
    fi
}

patch_refs() {
    local target="$1"
    local deps
    deps=$(otool -L "${target}" 2>/dev/null | grep -E '/(opt|usr/local)/homebrew|/Volumes/FastDisk/homebrew' | sed 's/^[[:space:]]*//' | awk '{print $1}' || true)
    for old_path in ${deps}; do
        local new_path=""
        if echo "${old_path}" | grep -q '\.framework/'; then
            local fw_name
            fw_name=$(echo "${old_path}" | sed 's|.*/\([^/]*\)\.framework.*|\1|')
            new_path="@executable_path/../Frameworks/${fw_name}.framework/Versions/5/${fw_name}"
        else
            local fname
            fname=$(basename "${old_path}")
            if [ -f "${FW}/${fname}" ]; then
                new_path="@executable_path/../Frameworks/${fname}"
            fi
        fi
        if [ -n "${new_path}" ]; then
            install_name_tool -change "${old_path}" "${new_path}" "${target}" 2>/dev/null || true
        fi
    done
}

ALL_FILES=$(collect_files)

for file in ${ALL_FILES}; do
    set_id "${file}"
done

for file in ${ALL_FILES}; do
    patch_refs "${file}"
done

patch_refs "${BINARY}"

echo "==> Step 5: Second pass for transitive deps..."
for file in ${ALL_FILES}; do
    patch_refs "${file}"
done
patch_refs "${BINARY}"

echo "==> Step 6: Ad-hoc code signing..."
codesign --force --deep --sign - "${APP}" 2>/dev/null || {
    echo "    (codesign failed, app will work but may need Gatekeeper bypass)"
}

echo "==> Step 7: Verification..."
BAD=0
for file in ${ALL_FILES} "${BINARY}"; do
    REFS=$(otool -L "${file}" 2>/dev/null | grep -E '/(opt|usr/local)/homebrew|/Volumes/FastDisk/homebrew' || true)
    if [ -n "${REFS}" ]; then
        echo "WARNING: $(echo "${file}" | sed "s|${APP}/||") has absolute refs:"
        echo "${REFS}"
        BAD=1
    fi
done

if [ "${BAD}" -eq 0 ]; then
    echo "All dependencies resolved (system frameworks + bundled)."
fi

APP_SIZE=$(du -sh "${APP}" | cut -f1)
echo ""
echo "==> Done! Distributable app bundle: ${APP} (${APP_SIZE})"

echo "==> Step 8: Creating DMG..."
DMG_NAME="MSHV.dmg"
hdiutil create -volname MSHV -srcfolder "${APP}" -ov "${DMG_NAME}"
DMG_SIZE=$(du -sh "${DMG_NAME}" | cut -f1)
echo ""
echo "==> Release ready: ${DMG_NAME} (${DMG_SIZE})"
