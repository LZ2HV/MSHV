#!/usr/bin/env bash
#
# Submit MSHV.app to Apple notarisation, wait for the result, staple the
# ticket, and produce a final distributable MSHV.app.zip in bin/.
#
# Run after `make standalone`:
#   make notarize
# or directly:
#   ./macos/notarize.sh
#
# Override the keychain credential profile name with:
#   MSHV_NOTARY_PROFILE=other-name ./macos/notarize.sh
#
# Setup required once per maintainer machine:
#   xcrun notarytool store-credentials MSHV-NOTARY \
#       --apple-id you@example.com \
#       --team-id ABCD123456 \
#       --password xxxx-xxxx-xxxx-xxxx
#
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
APP="$REPO/bin/MSHV.app"
ZIP="$REPO/bin/MSHV.app.zip"
PROFILE="${MSHV_NOTARY_PROFILE:-MSHV-NOTARY}"

if [[ ! -d "$APP" ]]; then
    echo "error: $APP not found — run 'make standalone' first." >&2
    exit 1
fi

# Refuse to submit an ad-hoc-signed bundle. Apple will reject it anyway,
# but the message is clearer if we catch it here.
if codesign -dvv "$APP" 2>&1 | grep -q "Signature=adhoc"; then
    echo "error: $APP is ad-hoc signed, not signed with a Developer ID." >&2
    echo "       Notarisation requires a 'Developer ID Application' cert" >&2
    echo "       in the login keychain. See macos/build-standalone.sh." >&2
    exit 1
fi

echo "==> zipping bundle for submission"
rm -f "$ZIP"
ditto -c -k --keepParent "$APP" "$ZIP"
ls -lh "$ZIP"

echo "==> submitting to Apple notarisation (profile: $PROFILE)"
echo "    this typically takes 1-15 minutes; --wait blocks until Apple replies."
xcrun notarytool submit "$ZIP" --keychain-profile "$PROFILE" --wait

echo "==> stapling notarisation ticket to $APP"
xcrun stapler staple "$APP"

echo "==> verifying staple"
xcrun stapler validate "$APP"
spctl --assess --type execute --verbose "$APP" || true

echo "==> re-zipping with stapled ticket"
rm -f "$ZIP"
ditto -c -k --keepParent "$APP" "$ZIP"
ls -lh "$ZIP"
shasum -a 256 "$ZIP"

echo
echo "==> done. distribute $ZIP — Gatekeeper will accept it offline."
