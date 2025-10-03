#!/bin/bash

# Kalshi Trader Build Script
# This script builds the project using CMake

set -e  # Exit on any error

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
VCPKG_TOOLCHAIN="/Users/jiminryu/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake"

echo "🔨 Building Kalshi Trader..."
echo "Project root: $PROJECT_ROOT"

# Check if build directory exists, if not configure the project
if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "📋 Configuring project (first time build)..."
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN"
fi

# Build the project
echo "⚙️  Building project..."
cmake --build build --config Release

echo "Build completed successfully!"
echo "Run the application with: ./build/src/kalshi_trader_app"