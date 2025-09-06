package types

// CompressionType represents different compression algorithms
type CompressionType int

const (
	CompressionRaw CompressionType = iota
	CompressionJPEGLossy
	CompressionJPEGLossless
	CompressionJPEG2000Lossy
	CompressionJPEG2000Lossless
	CompressionJPEGLSLossy
	CompressionJPEGLSLossless
	CompressionRLE
	CompressionDeflated
)
