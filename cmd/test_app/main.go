package main

import (
	"fmt"

	"github.com/caio-sobreiro/gdcmconv-go/internal/gdcm"
)

func main() {
	err := gdcm.ConvertToJPEG2000("input.dcm", "output.dcm")
	if err != nil {
		fmt.Println("Error:", err)
	}
}
