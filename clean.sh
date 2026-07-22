#!/usr/bin/env bash
# clean.sh — remove build artifacts
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

if [ -d "${BUILD_DIR}" ]; then
    echo "Removing ${BUILD_DIR}..."
    rm -rf "${BUILD_DIR}"
    echo "Done."
else
    echo "Nothing to clean — build directory not found."
fi
