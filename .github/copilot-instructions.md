# gdcmconv-go Copilot Instructions

## Big Picture
- Library wraps the C++ GDCM toolkit for DICOM transcoding; public Go API in `gdcmconv.go` calls CGO layer in `internal/gdcm` which bridges into `internal/gdcm/gdcmconv.cpp`.
- Static `third_party/prebuilt/<os_arch>` libs + headers mirror vendored `third_party/gdcm`; Go builds assume the right folder exists for the current `GOOS/GOARCH`.
- `internal/types` holds enums shared with C++; keep ordering aligned with the `compression_type_t` enum in `gdcmconv.cpp`.

## Key Files & Flows
- `gdcmconv.go` exports convenience helpers (`ConvertToJPEG2000`, etc.) that wrap `gdcm.ConvertImage` with preset options.
- `internal/gdcm/gdcmconv.go` marshals Go structs into the C bridge, translating errors from `gdcm_result_t`.
- `internal/gdcm/gdcmconv.cpp` mirrors the historical `gdcmconv` CLI behavior; codec selection and metadata tweaks live here, so changes to compression logic belong in this switch.
- Sample executions live in `cmd/sample_code` and `cmd/multi_platform_test`; they rely on the root package API plus the sample `input.dcm` in repo root.

## Build & Tooling
- Always build with CGO enabled: `CGO_ENABLED=1 go build ./...`.
- Rebuild vendored libs with `./scripts/build_gdcm.sh` (requires cmake + C++ toolchain); script copies `.a` outputs into `third_party/prebuilt/<os_arch>` and refreshes headers under `third_party/prebuilt/include`.
- For cross-arch smoke tests run inside Docker: `docker compose run --rm test ./scripts/build_gdcm.sh` then `go run ./cmd/multi_platform_test`.

## Patterns & Conventions
- New compression modes demand updates in three places: `types.Compression...` constants, the CGO switch in `internal/gdcm/gdcmconv.go`, and codec handling in `gdcmconv.cpp`.
- `ConvertOptions.QualityOrRate` is overloaded: JPEG uses quality 0-100, JPEG-LS consumes an integer error tolerance, JPEG2000 treats values as rate/quality; document expectations when adding APIs.
- C layer returns fixed-size error buffers; prefer concise error strings to avoid truncation.
- Metadata fingerprints (Implementation UID/Version/AE Title) are pinned in C++; changing them will break fixture hash comparisons.

## Testing & Validation
- No automated tests yet; manual validation uses the provided `input.dcm`. Example: `go run ./cmd/sample_code` emits multiple outputs for spot-checking.
- When tweaking the C++ bridge, verify generated binaries still link on darwin/arm64 and linux/arm64 by rebuilding prebuilt artifacts and running `file third_party/prebuilt/<os_arch>/libgdcmMSFF.a` to confirm architecture.

## Extending Safely
- Keep CGO flags in `internal/gdcm/gdcmconv.go` synchronized with the `third_party/prebuilt` folder layout if you add new platforms.
- Avoid deleting `third_party/gdcm`; upstream sync should be additive so prebuilt rebuilds continue to succeed.
- Document any new workflows or commands in `README.md` plus this guide to keep future agents aligned.
