#!/bin/bash
# =============================================================================
# TrussC Project Generator Build Script (macOS)
# =============================================================================
# Double-click this script to build projectGenerator
#
# NOTE: If macOS blocks this script, right-click and select "Open"
# =============================================================================

# Move to script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "  TrussC Project Generator Build Script"
echo "=========================================="
echo ""

# Source directory
SOURCE_DIR="$SCRIPT_DIR/../examples/tools/projectGenerator"

# Create build folder
if [ ! -d "$SOURCE_DIR/build" ]; then
    echo "Creating build directory..."
    mkdir -p "$SOURCE_DIR/build"
fi

cd "$SOURCE_DIR/build"

# CMake configuration
echo "Running CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: CMake configuration failed!"
    echo "Please make sure CMake is installed."
    echo "  brew install cmake"
    echo ""
    read -p "Press Enter to close..."
    exit 1
fi

# Build
echo ""
echo "Building..."
cmake --build .
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    echo ""
    read -p "Press Enter to close..."
    exit 1
fi

# Copy binary to distribution folder
echo ""
echo "Copying to distribution folder..."
rm -rf "$SCRIPT_DIR/projectGenerator.app"
cp -R "$SOURCE_DIR/bin/projectGenerator.app" "$SCRIPT_DIR/"

echo ""
echo "=========================================="
echo "  Build completed successfully!"
echo "=========================================="
echo ""
echo "Launching projectGenerator..."
open "$SCRIPT_DIR/projectGenerator.app"
