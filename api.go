package gdcmconv

import "github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"

// ConvertImage applies a compression transform according to the provided options.
func ConvertImage(inputPath, outputPath string, opts ConvertOptions) error {
	return gdcm.ConvertImage(inputPath, outputPath, opts)
}

// ConvertTransferSyntax rewrites the input DICOM using the requested transfer syntax flags.
func ConvertTransferSyntax(inputPath, outputPath string, explicitTS, implicitTS bool) error {
	return gdcm.ConvertTransferSyntax(inputPath, outputPath, explicitTS, implicitTS)
}

// ApplyLUT applies the modality LUT transformation and optionally upsamples to RGB8.
func ApplyLUT(inputPath, outputPath string, rgb8 bool) error {
	return gdcm.ApplyLUT(inputPath, outputPath, rgb8)
}

// RemoveTags strips private, retired, or group length elements based on the provided options.
func RemoveTags(inputPath, outputPath string, opts RemoveTagsOptions) error {
	return gdcm.RemoveTags(inputPath, outputPath, opts)
}
