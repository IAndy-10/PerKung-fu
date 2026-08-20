#!/bin/bash
set -e

PLUGIN_NAME="PerKungFu"
PRODUCT_NAME="PerKung-fu"
VERSION="0.1.0"
BUILD_DIR="build"
RELEASE_NAME="$PRODUCT_NAME-v$VERSION"
RELEASE_DIR="/tmp/$RELEASE_NAME"

echo "Building release: $RELEASE_NAME"

# Clean build
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR" && cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
cd ..

# Collect binaries
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR/VST3" "$RELEASE_DIR/AU"
cp -r "$BUILD_DIR/${PLUGIN_NAME}_artefacts/Release/Standalone/${PRODUCT_NAME}.app" "$RELEASE_DIR/"
cp -r ~/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3 "$RELEASE_DIR/VST3/"
cp -r ~/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component "$RELEASE_DIR/AU/"

# Create zip
cd /tmp
zip -r "$RELEASE_NAME.zip" "$RELEASE_NAME"
echo "Release ready: /tmp/$RELEASE_NAME.zip"
