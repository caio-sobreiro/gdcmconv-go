package gdcmconv

import (
	"fmt"

	"github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"
)

// ConvertImage applies a compression transform according to the provided options.
func ConvertImage(inputPath, outputPath string, opts ConvertOptions) error {
	return gdcm.ConvertImage(inputPath, outputPath, opts)
}

// ConvertDatasetTransferSyntax rewrites dataset metadata to the requested little-endian
// transfer-syntax preset (currently Implicit or Explicit VR Little Endian) without modifying
// or recompressing the pixel data buffer.
func ConvertDatasetTransferSyntax(inputPath, outputPath string, preset TransferSyntaxPreset) error {
	switch preset {
	case TransferSyntaxImplicitVRLittleEndian:
		return gdcm.ConvertTransferSyntax(inputPath, outputPath, false, true)
	case TransferSyntaxExplicitVRLittleEndian:
		return gdcm.ConvertTransferSyntax(inputPath, outputPath, true, false)
	default:
		return fmt.Errorf("unsupported transfer syntax preset: %d", preset)
	}
}

// ApplyLUT applies the modality LUT transformation and optionally upsamples to RGB8.
func ApplyLUT(inputPath, outputPath string, rgb8 bool) error {
	return gdcm.ApplyLUT(inputPath, outputPath, rgb8)
}

// RemoveTags strips private, retired, or group length elements based on the provided options.
func RemoveTags(inputPath, outputPath string, opts RemoveTagsOptions) error {
	return gdcm.RemoveTags(inputPath, outputPath, opts)
}

// ConvertToJPEG2000 is a convenience function for JPEG2000 lossless conversion
func ConvertToJPEG2000(inputPath string, outputPath string) error {
	return ConvertImage(inputPath, outputPath, ConvertOptions{
		Compression: CompressionJPEG2000Lossless,
	})
}

// ConvertToJPEG2000Lossy converts to JPEG2000 with specified quality
func ConvertToJPEG2000Lossy(inputPath string, outputPath string, quality float64) error {
	return ConvertImage(inputPath, outputPath, ConvertOptions{
		Compression:   CompressionJPEG2000Lossy,
		QualityOrRate: quality,
	})
}

// ConvertToJPEG converts to JPEG with specified quality
func ConvertToJPEG(inputPath string, outputPath string, quality float64) error {
	return ConvertImage(inputPath, outputPath, ConvertOptions{
		Compression:   CompressionJPEGLossy,
		QualityOrRate: quality,
	})
}

// ConvertToRaw decompresses the image to raw format
func ConvertToRaw(inputPath string, outputPath string) error {
	return ConvertImage(inputPath, outputPath, ConvertOptions{
		Compression: CompressionRaw,
	})
}
