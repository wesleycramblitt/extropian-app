#!/usr/bin/env bash
# test.sh — build and run the full test suite via CTest
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Build (includes test targets)
echo "=== Building ==="
"${SCRIPT_DIR}/build.sh"

echo
echo "=== Running tests ==="
ctest --test-dir "${BUILD_DIR}" --output-on-failure "$@"
