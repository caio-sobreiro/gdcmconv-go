#!/bin/bash
set -euo pipefail

GDCM_SRC_DIR="third_party/gdcm"
BUILD_DIR="build/gdcm"

mkdir -p "$BUILD_DIR"

cmake -S "$GDCM_SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DGDCM_BUILD_SHARED_LIBS=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DGDCM_BUILD_APPLICATIONS=OFF \
  -DGDCM_BUILD_EXAMPLES=OFF \
  -DGDCM_BUILD_TESTING=ON

build_jobs=${BUILD_JOBS:-4}
cmake --build "$BUILD_DIR" --parallel "$build_jobs"

arch=$(uname -m)
os=$(uname -s | tr '[:upper:]' '[:lower:]')
echo "Current OS/ARCH: $os/$arch"

# Copy necessary .a files to prebuilt directory for darwin/arm64
lib_dir="third_party/prebuilt/${os}_${arch}"
include_dir="third_party/prebuilt/include/${os}_${arch}"

mkdir -p "$lib_dir"
cp build/gdcm/bin/libgdcm*.a "$lib_dir"/
if [ -f build/gdcm/bin/libsocketxx.a ]; then
  cp build/gdcm/bin/libsocketxx.a "$lib_dir"/
fi

rm -rf "$include_dir"
mkdir -p "$include_dir"

find third_party/gdcm/Source -name "*.h" -exec cp {} "$include_dir"/ \;
find third_party/gdcm/Source -name "*.txx" -exec cp {} "$include_dir"/ \;
find build/gdcm/Source/Common -name '*.h' -exec cp {} "$include_dir"/ \;
find build/gdcm/Source/Common -name '*.txx' -exec cp {} "$include_dir"/ \;

# Cleanup
rm -rf build/
