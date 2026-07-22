#!/usr/bin/env bash
# build.sh — configure and build extropian-app (and tests)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
GENERATOR="${GENERATOR:-Ninja}"

echo "=== extropian-app build ==="
echo "  Build type : ${BUILD_TYPE}"
echo "  Generator  : ${GENERATOR}"
echo "  Build dir  : ${BUILD_DIR}"
echo

cmake -S "${SCRIPT_DIR}" \
      -B "${BUILD_DIR}" \
      -G "${GENERATOR}" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      "$@"

cmake --build "${BUILD_DIR}"
