package main

import (
	"fmt"

	"github.com/caio-sobreiro/gdcmconv-go"
	"github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"
	"github.com/caio-sobreiro/gdcmconv-go/internal/types"
)

func main() {
	// Example 1: Simple JPEG2000 lossless conversion (backwards compatible)
	fmt.Println("Converting to JPEG2000 lossless...")
	err := gdcmconv.ConvertToJPEG2000("input.dcm", "output-j2k-lossless-go.dcm")
	if err != nil {
		fmt.Printf("JPEG2000 lossless conversion failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ JPEG2000 lossless conversion successful")
	}

	// Example 2: JPEG2000 lossy conversion with quality setting
	fmt.Println("Converting to JPEG2000 lossy...")
	err = gdcmconv.ConvertToJPEG2000Lossy("input.dcm", "output-j2k-lossy-go.dcm", 50.0)
	if err != nil {
		fmt.Printf("JPEG2000 lossy conversion failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ JPEG2000 lossy conversion successful")
	}

	// Example 3: JPEG conversion with quality setting
	fmt.Println("Converting to JPEG...")
	err = gdcmconv.ConvertToJPEG("input.dcm", "output-jpeg-go.dcm", 80.0)
	if err != nil {
		fmt.Printf("JPEG conversion failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ JPEG conversion successful")
	}

	// Example 4: Decompress to raw format
	fmt.Println("Converting to raw format...")
	err = gdcmconv.ConvertToRaw("input.dcm", "output-raw-go.dcm")
	if err != nil {
		fmt.Printf("Raw conversion failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ Raw conversion successful")
	}

	// Example 5: Using the generic ConvertImage function with custom options
	fmt.Println("Converting with custom options...")
	err = gdcm.ConvertImage("input.dcm", "output-custom-go.dcm", gdcm.ConvertOptions{
		Compression:   types.CompressionRLE,
		QualityOrRate: 0, // Not used for RLE
	})
	if err != nil {
		fmt.Printf("Custom conversion failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ Custom RLE conversion successful")
	}

	// Example 6: Remove private tags
	fmt.Println("Removing private tags...")
	err = gdcm.RemoveTags("input.dcm", "output-no-private-go.dcm", gdcm.RemoveTagsOptions{
		RemovePrivate:     true,
		RemoveRetired:     false,
		RemoveGroupLength: false,
	})
	if err != nil {
		fmt.Printf("Tag removal failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ Private tag removal successful")
	}

	// Example 7: Apply LUT transformation
	fmt.Println("Applying LUT transformation...")
	err = gdcm.ApplyLUT("input.dcm", "output-lut-go.dcm", false)
	if err != nil {
		fmt.Printf("LUT application failed: %s\n", err.Error())
	} else {
		fmt.Println("✓ LUT transformation successful")
	}

	fmt.Println("\nAll conversions completed!")
}
