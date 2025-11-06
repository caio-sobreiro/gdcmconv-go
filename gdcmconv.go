package gdcmconv

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
