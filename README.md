# gdcmconv-go

`gdcmconv-go` is a CGO-based Go library that provides a high-level wrapper around the [GDCM](https://github.com/malaterre/GDCM) library. It enables fast, robust, and production-grade transcoding (compression and decompression) of DICOM images directly from Go code.

## Features

- **Transcode DICOM images**: Compress and decompress DICOM files using industry-standard codecs (JPEG, JPEG2000, RLE, etc.)
- **Lossless and lossy compression**: Support for JPEG, JPEG-LS, JPEG2000, RLE, and raw (uncompressed) formats
- **LUT (Lookup Table) operations**: Apply LUT transformations to DICOM images
- **Transfer syntax normalization**: Rewrite dataset metadata between implicit/explicit VR little-endian variants without touching pixel data
- **High performance**: Leverages the mature and optimized GDCM C++ library

## Roadmap
- [x] Multi-platform static builds for macOS amd64/arm64 + Linux amd64/arm64
- [x] Public Go API with reusable Convert/Transfer/LUT/Tag helpers
- [x] Integration-style smoke tests covering core flows (`go test ./...`)
- [ ] Windows support
- [ ] Expand fixtures to cover JPEG + LUT workflows
- [ ] Add CI to exercise cross-platform builds and test runs
- [ ] Publish performance benchmarks

## Installation

This package requires the ability to build CGO projects, and pre-built GDCM object files for supported OS/ARCH combinations are included in this repository.

```sh
go get github.com/caio-sobreiro/gdcmconv-go
```

## Example Usage

```go
package main

import (
	"github.com/caio-sobreiro/gdcmconv-go"
)

func main() {
	gdcmconv.ConvertToJPEG2000("input.dcm", "output.dcm")
}

```

### Advanced Usage

For more control, use the advanced API:

```go
import "github.com/caio-sobreiro/gdcmconv-go"

// Compress to JPEG2000 lossy with quality 50
err := gdcmconv.ConvertImage("input.dcm", "output.dcm", gdcmconv.ConvertOptions{
    Compression:   gdcmconv.CompressionJPEG2000Lossy,
    QualityOrRate: 50.0,
})

// Decompress to raw
err = gdcmconv.ConvertToRaw("input.dcm", "output.dcm")

// Change transfer syntax to Explicit VR Little Endian without re-encoding pixel data
err = gdcmconv.ConvertDatasetTransferSyntax(
    "input.dcm",
    "output-explicit.dcm",
    gdcmconv.TransferSyntaxExplicitVRLittleEndian,
)

// Remove private tags
err = gdcmconv.RemoveTags("input.dcm", "output-stripped.dcm", gdcmconv.RemoveTagsOptions{
    RemovePrivate:     true,
    RemoveRetired:     true,
    RemoveGroupLength: true,
})
```

## Supported Compression Types

- `CompressionRaw` (uncompressed)
- `CompressionJPEGLossy`
- `CompressionJPEGLossless`
- `CompressionJPEG2000Lossy`
- `CompressionJPEG2000Lossless`
- `CompressionJPEGLSLossy`
- `CompressionJPEGLSLossless`
- `CompressionRLE`
- `CompressionDeflated`

## Why use gdcmconv-go?

- **Production-grade**: Built on top of the widely used GDCM C++ library
- **Performance**: Native code for fast transcoding
- **Simplicity**: Go API abstracts away C++ and command-line details
- **Flexibility**: Supports a wide range of DICOM compression and transformation workflows
- **Tested flows**: Integration tests validate core conversions by default (`go test ./...`)

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
