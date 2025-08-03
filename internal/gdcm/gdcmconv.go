package gdcm

/*
#cgo CXXFLAGS: -std=c++11
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/Attribute
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/Common
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/DataDictionary
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/DataStructureAndEncodingDefinition
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/MediaStorageAndFileFormat
#cgo CXXFLAGS: -I${SRCDIR}/../../third_party/gdcm/Source/MessageExchangeDefinition

#cgo CXXFLAGS: -I${SRCDIR}/../../build/gdcm/Source/Common

#cgo LDFLAGS: -L${SRCDIR}/../../build/gdcm/bin
#cgo LDFLAGS: -lgdcmMSFF -lgdcmDICT -lgdcmCommon -lgdcmuuid
#cgo LDFLAGS: -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmopenjp2 -lgdcmDSED -lgdcmexpat -lgdcmIOD
#cgo LDFLAGS: -lgdcmMEXD -lgdcmzlib -lsocketxx -lgdcmcharls

#include <stdlib.h>

extern int c_main(int argc, char *argv[]);
*/
import "C"
import (
	"fmt"
	"unsafe"
)

func ConvertToJPEG2000(inputPath string, outputPath string) error {
	argv := []string{"gdcmconv", "--j2k", inputPath, outputPath}
	argc := C.int(len(argv))

	cArgv := make([]*C.char, len(argv))
	for i, arg := range argv {
		cArgv[i] = C.CString(arg)
		defer C.free(unsafe.Pointer(cArgv[i]))
	}

	result := C.c_main(argc, (**C.char)(unsafe.Pointer(&cArgv[0])))
	if result != 0 {
		return fmt.Errorf("failed to convert DICOM file to JPEG2000")
	}
	return nil
}
