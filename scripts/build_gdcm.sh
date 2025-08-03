#!/bin/bash
set -e

GDCM_SRC_DIR="../../third_party/gdcm"
BUILD_DIR="../../build/gdcm"

mkdir -p "$BUILD_DIR"

cmake -S "$GDCM_SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DGDCM_BUILD_SHARED_LIBS=OFF \
  -DGDCM_BUILD_APPLICATIONS=OFF \
  -DGDCM_BUILD_EXAMPLES=OFF \
  -DGDCM_BUILD_TESTING=OFF

cmake --build "$BUILD_DIR" --parallel
