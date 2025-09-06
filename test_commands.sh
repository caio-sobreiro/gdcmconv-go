#!/bin/bash

# Equivalent gdcmconv commands for the test app conversions
# Run these to generate reference outputs for SHA256 comparison

echo "Running equivalent gdcmconv commands..."

# Example 1: Simple JPEG2000 lossless conversion
echo "1. JPEG2000 lossless conversion..."
gdcmconv --j2k input.dcm output-j2k-lossless-ref.dcm

# Example 2: JPEG2000 lossy conversion with quality setting (50.0)
echo "2. JPEG2000 lossy conversion (quality 50.0)..."
gdcmconv --j2k --lossy --quality 50.0 input.dcm output-j2k-lossy-ref.dcm

# Example 3: JPEG conversion with quality setting (80.0)
echo "3. JPEG conversion (quality 80.0)..."
gdcmconv --jpeg --lossy --quality 80.0 input.dcm output-jpeg-ref.dcm

# Example 4: Decompress to raw format
echo "4. Raw format conversion..."
gdcmconv --raw input.dcm output-raw-ref.dcm

# Example 5: RLE compression
echo "5. RLE compression..."
gdcmconv --rle input.dcm output-custom-ref.dcm

# Example 6: Remove private tags
echo "6. Remove private tags..."
gdcmconv --remove-private-tags input.dcm output-no-private-ref.dcm

# Example 7: Apply LUT transformation (not RGB8)
echo "7. Apply LUT transformation..."
gdcmconv --apply-lut input.dcm output-lut-ref.dcm

echo "Reference files generated. Now run the Go test app and compare..."

# Generate SHA256 checksums for comparison
echo ""
echo "=== Reference file checksums (gdcmconv) ==="

./test_app

# Compare the hash of each reference file with the corresponding output from the Go app
echo ""
echo "=== Comparing Go output vs reference (gdcmconv) ==="

for ref in output-*-ref.dcm; do
	gofile="${ref/-ref/-go}"
	if [ -f "$gofile" ]; then
		hash_ref=$(sha256sum "$ref" | awk '{print $1}')
		hash_go=$(sha256sum "$gofile" | awk '{print $1}')
		   if [ "$hash_ref" = "$hash_go" ]; then
			   # Green
			   echo -e "\033[32mPASS: $gofile matches $ref ($hash_ref)\033[0m"
		   else
			   # Red
			   echo -e "\033[31mFAIL: $gofile does NOT match $ref\033[0m"
			   echo -e "\033[31m  ref: $hash_ref\033[0m"
			   echo -e "\033[31m  go:  $hash_go\033[0m"
		   fi
	else
		echo "MISSING: $gofile (no Go output to compare)"
	fi
done
