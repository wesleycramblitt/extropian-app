#!/usr/bin/env bash
# run.sh — run unit tests (primary "run" target for a library project)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Ensure the project is built first
if [ ! -f "${BUILD_DIR}/exd-app-tests" ]; then
    echo "Tests not built yet — running build.sh first..."
    "${SCRIPT_DIR}/build.sh"
fi

echo "=== extropian-app tests ==="
echo

"${BUILD_DIR}/exd-app-tests" "$@"
