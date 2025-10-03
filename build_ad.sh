#!/bin/bash

# Kalshi Trader Build Script (Advanced)
# Usage: ./build.sh [options]
# Options:
#   clean    - Clean build directory and rebuild from scratch
#   run      - Build and run the application
#   debug    - Build in debug mode
#   help     - Show this help message

set -e  # Exit on any error

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
VCPKG_TOOLCHAIN="/Users/jiminryu/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Default build type
BUILD_TYPE="Release"

show_help() {
    echo "Kalshi Trader Build Script"
    echo ""
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  clean    Clean build directory and rebuild from scratch"
    echo "  run      Build and run the application"
    echo "  debug    Build in debug mode"
    echo "  help     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0           # Normal build"
    echo "  $0 clean     # Clean build"
    echo "  $0 run       # Build and run"
    echo "  $0 debug     # Debug build"
}

clean_build() {
    echo "🧹 Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo "✅ Build directory cleaned"
}

configure_project() {
    echo "📋 Configuring project..."
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
}

build_project() {
    echo "Building project in $BUILD_TYPE mode..."
    cmake --build build --config "$BUILD_TYPE"
    echo "Build completed successfully!"
}

run_application() {
    echo "Running application..."
    ./build/src/kalshi_trader_app
}

# Parse command line arguments
case "${1:-}" in
    "clean")
        clean_build
        configure_project
        build_project
        ;;
    "run")
        # Check if build directory exists, if not configure the project
        if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
            configure_project
        fi
        build_project
        run_application
        ;;
    "debug")
        BUILD_TYPE="Debug"
        # Check if build directory exists, if not configure the project
        if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
            configure_project
        fi
        build_project
        ;;
    "help"|"-h"|"--help")
        show_help
        exit 0
        ;;
    "")
        # Default: just build
        echo "🔨 Building Kalshi Trader..."
        echo "Project root: $PROJECT_ROOT"
        
        # Check if build directory exists, if not configure the project
        if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
            configure_project
        fi
        build_project
        echo "Run the application with: ./build/src/kalshi_trader_app"
        ;;
    *)
        echo "Unknown option: $1"
        echo ""
        show_help
        exit 1
        ;;
esac