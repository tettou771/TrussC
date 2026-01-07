#!/bin/bash
# =============================================================================
# TrussC Project Generator Build Script (Linux)
# =============================================================================
# Run this script to build projectGenerator
# Usage: ./buildProjectGenerator_linux.sh
# =============================================================================

# Move to script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "  TrussC Project Generator Build Script"
echo "=========================================="
echo ""

# Source directory
SOURCE_DIR="$SCRIPT_DIR/tools/projectGenerator"

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
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Fedora: sudo dnf install cmake"
    echo "  Arch: sudo pacman -S cmake"
    echo ""
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
    exit 1
fi

# Create symlink to binary in distribution folder
echo ""
echo "Creating symlink to distribution folder..."
rm -f "$SCRIPT_DIR/projectGenerator"
ln -s "$SOURCE_DIR/bin/projectGenerator" "$SCRIPT_DIR/projectGenerator"

echo ""
echo "=========================================="
echo "  Build completed successfully!"
echo "=========================================="
echo ""
echo "Launching projectGenerator..."
"$SCRIPT_DIR/projectGenerator"
