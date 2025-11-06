#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGETS=${TARGETS:-"linux/arm64 linux/amd64"}
IMAGE_BASE=${IMAGE_BASE:-gdcmconv-go/builder}
DOCKERFILE_PATH=${DOCKERFILE_PATH:-$ROOT_DIR}

for target in $TARGETS; do
  tag_suffix="${target//\//-}"
  image_tag="$IMAGE_BASE:$tag_suffix"

  echo "\n==> Building builder image for $target"
  docker buildx build --platform "$target" --load -t "$image_tag" "$DOCKERFILE_PATH"

  echo "==> Running GDCM build for $target"
  docker run --rm --platform "$target" \
    -v "$ROOT_DIR/third_party/prebuilt:/app/third_party/prebuilt" \
    -w /app \
    "$image_tag" bash -lc "./scripts/build_gdcm.sh"

done

echo "\nAll builds completed."
