package gdcm

/*
#cgo CXXFLAGS: -std=c++11
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/Attribute
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/Common
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/DataDictionary
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/DataStructureAndEncodingDefinition
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/MediaStorageAndFileFormat
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/MessageExchangeDefinition

#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/prebuilt/include

#cgo darwin,arm64 LDFLAGS: -L${SRCDIR}/../../third_party/prebuilt/darwin_arm64
#cgo linux,amd64 LDFLAGS: -L${SRCDIR}/../../third_party/prebuilt/linux_x86_64

#cgo LDFLAGS: -lgdcmMSFF -lgdcmDICT -lgdcmCommon -lgdcmIOD -lgdcmMEXD -lgdcmuuid
#cgo LDFLAGS: -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmopenjp2 -lgdcmDSED
#cgo LDFLAGS: -lgdcmexpat -lgdcmmd5 -lgdcmzlib -lsocketxx -lgdcmcharls -lpthread

#include <stdlib.h>

// Compression types
typedef enum {
    COMPRESSION_RAW = 0,
    COMPRESSION_JPEG_LOSSY,
    COMPRESSION_JPEG_LOSSLESS,
    COMPRESSION_JPEG2000_LOSSY,
    COMPRESSION_JPEG2000_LOSSLESS,
    COMPRESSION_JPEGLS_LOSSY,
    COMPRESSION_JPEGLS_LOSSLESS,
    COMPRESSION_RLE,
    COMPRESSION_DEFLATED
} compression_type_t;

// Result structure
typedef struct {
    int success;
    char error_message[256];
} gdcm_result_t;

// Function declarations
extern gdcm_result_t gdcm_convert_image(const char* input_file, const char* output_file,
                                       compression_type_t compression, double quality_or_rate);
extern gdcm_result_t gdcm_convert_transfer_syntax(const char* input_file, const char* output_file,
                                                 int explicit_ts, int implicit_ts);
extern gdcm_result_t gdcm_apply_lut(const char* input_file, const char* output_file, int rgb8);
extern gdcm_result_t gdcm_remove_tags(const char* input_file, const char* output_file,
                                     int remove_private, int remove_retired, int remove_group_length);

*/
import "C"
import (
	"fmt"
	"unsafe"

	"github.com/caio-sobreiro/gdcmconv-go/internal/types"
)

// ConvertOptions holds options for DICOM conversion
type ConvertOptions struct {
	Compression   types.CompressionType
	QualityOrRate float64 // For lossy compression: quality (0-100) or rate for JPEG2000
}

// ConvertImage converts a DICOM image with the specified compression
func ConvertImage(inputPath, outputPath string, opts ConvertOptions) error {
	cInputPath := C.CString(inputPath)
	defer C.free(unsafe.Pointer(cInputPath))

	cOutputPath := C.CString(outputPath)
	defer C.free(unsafe.Pointer(cOutputPath))

	result := C.gdcm_convert_image(
		cInputPath,
		cOutputPath,
		C.compression_type_t(opts.Compression),
		C.double(opts.QualityOrRate),
	)

	if result.success == 0 {
		return fmt.Errorf("conversion failed: %s", C.GoString(&result.error_message[0]))
	}

	return nil
}

// ConvertTransferSyntax changes the transfer syntax without compression
func ConvertTransferSyntax(inputPath, outputPath string, explicitTS, implicitTS bool) error {
	cInputPath := C.CString(inputPath)
	defer C.free(unsafe.Pointer(cInputPath))

	cOutputPath := C.CString(outputPath)
	defer C.free(unsafe.Pointer(cOutputPath))

	var explicitTSInt, implicitTSInt C.int
	if explicitTS {
		explicitTSInt = 1
	}
	if implicitTS {
		implicitTSInt = 1
	}

	result := C.gdcm_convert_transfer_syntax(
		cInputPath,
		cOutputPath,
		explicitTSInt,
		implicitTSInt,
	)

	if result.success == 0 {
		return fmt.Errorf("transfer syntax conversion failed: %s", C.GoString(&result.error_message[0]))
	}

	return nil
}

// ApplyLUT applies lookup table transformation
func ApplyLUT(inputPath, outputPath string, rgb8 bool) error {
	cInputPath := C.CString(inputPath)
	defer C.free(unsafe.Pointer(cInputPath))

	cOutputPath := C.CString(outputPath)
	defer C.free(unsafe.Pointer(cOutputPath))

	var rgb8Int C.int
	if rgb8 {
		rgb8Int = 1
	}

	result := C.gdcm_apply_lut(
		cInputPath,
		cOutputPath,
		rgb8Int,
	)

	if result.success == 0 {
		return fmt.Errorf("LUT application failed: %s", C.GoString(&result.error_message[0]))
	}

	return nil
}

// RemoveTagsOptions holds options for tag removal
type RemoveTagsOptions struct {
	RemovePrivate     bool
	RemoveRetired     bool
	RemoveGroupLength bool
}

// RemoveTags removes specified types of tags from DICOM file
func RemoveTags(inputPath, outputPath string, opts RemoveTagsOptions) error {
	cInputPath := C.CString(inputPath)
	defer C.free(unsafe.Pointer(cInputPath))

	cOutputPath := C.CString(outputPath)
	defer C.free(unsafe.Pointer(cOutputPath))

	var removePrivateInt, removeRetiredInt, removeGroupLengthInt C.int
	if opts.RemovePrivate {
		removePrivateInt = 1
	}
	if opts.RemoveRetired {
		removeRetiredInt = 1
	}
	if opts.RemoveGroupLength {
		removeGroupLengthInt = 1
	}

	result := C.gdcm_remove_tags(
		cInputPath,
		cOutputPath,
		removePrivateInt,
		removeRetiredInt,
		removeGroupLengthInt,
	)

	if result.success == 0 {
		return fmt.Errorf("tag removal failed: %s", C.GoString(&result.error_message[0]))
	}

	return nil
}
