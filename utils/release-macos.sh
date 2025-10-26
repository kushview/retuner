#!/usr/bin/sh

if [ ! -f "utils/codesign.env" ]; then
    echo "Error: codesign.env file not found. Please create this file with your code signing configuration."
    exit 1
fi

source utils/codesign.env

ENTITLEMENTS_PATH="plugin.entitlements"
PACKAGESBUILD_EXECUTABLE="/usr/local/bin/packagesbuild"
VERSION="1.0.0"
PKG_PROJECT_PATH="deploy/setup.pkgproj"

find deploy \( -name "*.pkg" -o -name "*.zip" \) -exec rm -f {} \;

rm -rf build
cmake -Bbuild . -GNinja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.13" \
    -DJUCE_COPY_PLUGIN_AFTER_BUILD=FALSE \
    "$@"
cmake --build build --config Release

if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

# Validate signatures on all plugin targets
echo "Signing code"

# Find all plugin bundles that should be signed
PLUGIN_BUNDLES=$(find build -name "*.app" -o -name "*.vst3" -o -name "*.lv2" -o -name "*.component" -o  -name "*.clap" | grep -E "(Release|Debug)" | head -20)

if [ -z "$PLUGIN_BUNDLES" ]; then
    echo "Error: No plugin bundles found to sign"
    exit 1
fi

VALIDATION_FAILED=0

get_main_binary() {
    local target="$1"
    if [ -d "$target" ]; then
        find "$target" -type f | while read -r f; do
            if file "$f" | grep -qE 'Mach-O|ELF'; then
                echo "$f"
                return
            fi
        done
    else
        if file "$target" | grep -qE 'Mach-O|ELF'; then
            echo "$target"
        fi
    fi
}

for bundle in $PLUGIN_BUNDLES; do
    echo "Validating: $bundle"

    # Special handling for LV2: find the .so binary inside the bundle
    if [[ "$bundle" == *.lv2 ]]; then
        SO_BINARY=$(find "$bundle" -name "*.so" | head -1)
        if [ -z "$SO_BINARY" ]; then
            echo "Warning: No .so binary found in LV2 bundle: $bundle"
            VALIDATION_FAILED=1
            continue
        fi
        TARGET="$SO_BINARY"
    else
        TARGET="$bundle"
    fi

    # Check if target exists
    if [ ! -e "$TARGET" ]; then
        echo "Warning: Target not found: $TARGET"
        VALIDATION_FAILED=1
        continue
    fi

    # Sign the target
    codesign --force --deep --sign "${CODE_SIGN_IDENTITY}" --timestamp --options runtime \
        --entitlements "${ENTITLEMENTS_PATH}" "$TARGET"

    # Validate the signature
    if codesign --verify --deep --strict "$TARGET" 2>/dev/null; then
        echo "✓ Valid signature: $TARGET"
    else
        echo "✗ Invalid or missing signature: $TARGET"
        VALIDATION_FAILED=1
    fi

    # Check if target is a universal binary (contains both x86_64 and arm64)
    if [ -d "$TARGET" ]; then
        # If target is a bundle directory, find main binary inside
        MAIN_BINARY=$(get_main_binary "$TARGET")
        if [ -n "$MAIN_BINARY" ]; then
            ARCHS=$(lipo -archs "$MAIN_BINARY" 2>/dev/null)
        else
            ARCHS=""
        fi
    else
        ARCHS=$(lipo -archs "$TARGET" 2>/dev/null)
    fi
    # if [[ "$ARCHS" != *"x86_64"* || "$ARCHS" != *"arm64"* ]]; then
    #     echo "✗ Not universal binary: $TARGET (found: $ARCHS)"
    #     VALIDATION_FAILED=1
    # fi
done

# Exit if any validation failed
if [ $VALIDATION_FAILED -eq 1 ]; then
    echo "Error: Code signature validation failed for one or more plugin bundles"
    echo "Please fix signing issues before creating installer"
    exit 1
fi

set -e

# Artifacts zip
echo "Making the zip file"
python3 utils/artifacts.py

echo "Making the isntaller"
packagesbuild --verbose --project "${PKG_PROJECT_PATH}"
productsign --sign "${INSTALLER_SIGN_IDENTITY}" --timestamp \
    "deploy/retuner-macos-${VERSION}_unsigned.pkg" \
    "deploy/retuner-macos-${VERSION}.pkg"
pkgutil --check-signature "deploy/retuner-macos-${VERSION}.pkg"

echo "Notarizing"
xcrun notarytool submit \
    "deploy/retuner-macos-${VERSION}.pkg" \
    --keychain-profile "notarytool-profile" \
    --wait
xcrun notarytool submit \
    "deploy/retuner-macos-${VERSION}.zip" \
    --keychain-profile "notarytool-profile" \
    --wait

echo "Stapling notarization ticket..."
xcrun stapler staple "deploy/retuner-macos-${VERSION}.pkg"
xcrun stapler validate "deploy/retuner-macos-${VERSION}.pkg"
