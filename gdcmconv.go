package gdcmconv

import (
	"github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"
)

func Convert(inputFile string, outputFile string) error {
	return gdcm.ConvertToJPEG2000(inputFile, outputFile)
}
