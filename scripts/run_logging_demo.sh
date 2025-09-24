#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR="${BUILD_DIR:-build}"
mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j
TSLOG_LEVEL="${TSLOG_LEVEL:-INFO}" "${BUILD_DIR}/log_demo" "${THREADS:-8}" "${LINES:-200}"