#!/bin/bash
set -e

GDCM_SRC_DIR="third_party/gdcm"
BUILD_DIR="build/gdcm"

mkdir -p "$BUILD_DIR"

cmake -S "$GDCM_SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DGDCM_BUILD_SHARED_LIBS=OFF \
  -DGDCM_BUILD_APPLICATIONS=OFF \
  -DGDCM_BUILD_EXAMPLES=OFF \
  -DGDCM_BUILD_TESTING=OFF

cmake --build "$BUILD_DIR" --parallel

# Copy necessary .a files to prebuilt directory for darwin/arm64
cp build/gdcm/bin/libgdcm*.a third_party/prebuilt/darwin_arm64/

# Copy generated header files
mkdir -p third_party/prebuilt/darwin_arm64/include

find third_party/gdcm/Source -name "*.h" | xargs -I {} sh -c 'cp {} third_party/prebuilt/darwin_arm64/include/$(basename {})'
find build/gdcm/Source/Common -name '*.h' | xargs -I {} sh -c 'cp {} third_party/prebuilt/darwin_arm64/include/$(basename {})'
