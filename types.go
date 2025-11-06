package gdcmconv

import (
	"github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"
	"github.com/caio-sobreiro/gdcmconv-go/internal/types"
)

// CompressionType represents the compression codec used during conversion.
type CompressionType = types.CompressionType

const (
	CompressionRaw              CompressionType = types.CompressionRaw
	CompressionJPEGLossy        CompressionType = types.CompressionJPEGLossy
	CompressionJPEGLossless     CompressionType = types.CompressionJPEGLossless
	CompressionJPEG2000Lossy    CompressionType = types.CompressionJPEG2000Lossy
	CompressionJPEG2000Lossless CompressionType = types.CompressionJPEG2000Lossless
	CompressionJPEGLSLossy      CompressionType = types.CompressionJPEGLSLossy
	CompressionJPEGLSLossless   CompressionType = types.CompressionJPEGLSLossless
	CompressionRLE              CompressionType = types.CompressionRLE
	CompressionDeflated         CompressionType = types.CompressionDeflated
)

// ConvertOptions configures how ConvertImage operates.
type ConvertOptions = gdcm.ConvertOptions

// RemoveTagsOptions configures how RemoveTags behaves.
type RemoveTagsOptions = gdcm.RemoveTagsOptions
