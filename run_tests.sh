#!/bin/bash
# Chronos Test Runner Script
# This script builds and runs the Chronos test suite with various configurations

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
TEST_MODE="${1:-all}"

print_header() {
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}$1${NC}"
    echo -e "${GREEN}================================${NC}"
}

print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}WARNING: $1${NC}"
}

# Clean build directory
clean_build() {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    mkdir -p "$BUILD_DIR"
}

# Run tests with standard configuration
run_standard_tests() {
    print_header "Running Standard Tests"
    clean_build
    cd "$BUILD_DIR"
    cmake -DCHRONOS_BUILD_TESTS=ON ..
    cmake --build . -j$(nproc)
    ctest --output-on-failure --verbose
    cd ..
}

# Run tests with AddressSanitizer
run_asan_tests() {
    print_header "Running Tests with AddressSanitizer"
    clean_build
    cd "$BUILD_DIR"
    cmake -DENABLE_ASAN=ON -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . -j$(nproc)
    ctest --output-on-failure --verbose
    cd ..
}

# Run tests with ThreadSanitizer
run_tsan_tests() {
    print_header "Running Tests with ThreadSanitizer"
    clean_build
    cd "$BUILD_DIR"
    cmake -DENABLE_TSAN=ON -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . -j$(nproc)
    ctest --output-on-failure --verbose
    cd ..
}

# Run release mode tests
run_release_tests() {
    print_header "Running Release Mode Tests"
    clean_build
    cd "$BUILD_DIR"
    cmake -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -j$(nproc)
    ctest --output-on-failure --verbose
    cd ..
}

# Run debug mode tests
run_debug_tests() {
    print_header "Running Debug Mode Tests"
    clean_build
    cd "$BUILD_DIR"
    cmake -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . -j$(nproc)
    ctest --output-on-failure --verbose
    cd ..
}

# Main execution
case "$TEST_MODE" in
    "all")
        echo "Running all test configurations..."
        run_standard_tests
        run_asan_tests
        run_tsan_tests
        run_release_tests
        print_header "All Tests Completed Successfully!"
        ;;
    "standard")
        run_standard_tests
        ;;
    "asan")
        run_asan_tests
        ;;
    "tsan")
        run_tsan_tests
        ;;
    "release")
        run_release_tests
        ;;
    "debug")
        run_debug_tests
        ;;
    "quick")
        print_header "Running Quick Test (Debug mode only)"
        run_debug_tests
        ;;
    *)
        echo "Usage: $0 [all|standard|asan|tsan|release|debug|quick]"
        echo ""
        echo "Test modes:"
        echo "  all      - Run all test configurations (standard, asan, tsan, release)"
        echo "  standard - Run standard tests"
        echo "  asan     - Run tests with AddressSanitizer"
        echo "  tsan     - Run tests with ThreadSanitizer"
        echo "  release  - Run tests in release mode"
        echo "  debug    - Run tests in debug mode"
        echo "  quick    - Run quick tests (debug mode only)"
        exit 1
        ;;
esac

exit 0