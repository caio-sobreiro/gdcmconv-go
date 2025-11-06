package main

import (
	"fmt"
	"runtime"

	"github.com/caio-sobreiro/gdcmconv-go"
)

func main() {
	fmt.Println("Converting to JPEG2000 lossless...")
	err := gdcmconv.ConvertToJPEG2000("input.dcm", "output.dcm")
	if err != nil {
		fmt.Println("Error:", err)
	}

	fmt.Println("Conversion completed successfully")

	// print os and arch
	fmt.Println()
	fmt.Println("Running on OS:", runtime.GOOS)
	fmt.Println("Running on Arch:", runtime.GOARCH)
}
