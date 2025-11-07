package gdcmconv

import (
	"errors"
	"os"
	"path/filepath"
	"testing"
)

const skipEnv = "GDCM_SKIP_INTEGRATION_TESTS"

func requireCGO(t *testing.T) {
	t.Helper()
	if value, ok := os.LookupEnv("CGO_ENABLED"); ok && value == "0" {
		t.Skip("skipping integration tests because CGO is disabled")
	}
}

func requireSampleDicom(t *testing.T) string {
	t.Helper()
	if os.Getenv(skipEnv) == "1" {
		t.Skip("skipping integration tests via environment override")
	}

	path := filepath.Join("input.dcm")
	if _, err := os.Stat(path); err != nil {
		if errors.Is(err, os.ErrNotExist) {
			t.Skip("input.dcm not found; skipping integration tests")
		}
		t.Fatalf("failed to stat input.dcm: %v", err)
	}

	return path
}

func TestConvenienceConverters(t *testing.T) {
	requireCGO(t)
	input := requireSampleDicom(t)

	outDir := t.TempDir()

	tests := []struct {
		name string
		run  func(in, out string) error
	}{
		{
			name: "JPEG2000Lossless",
			run: func(in, out string) error {
				return ConvertToJPEG2000(in, out)
			},
		},
		{
			name: "JPEG2000Lossy",
			run: func(in, out string) error {
				return ConvertToJPEG2000Lossy(in, out, 50)
			},
		},
		{
			name: "Raw",
			run: func(in, out string) error {
				return ConvertToRaw(in, out)
			},
		},
	}

	for _, tc := range tests {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			output := filepath.Join(outDir, tc.name+".dcm")
			if err := tc.run(input, output); err != nil {
				t.Fatalf("conversion failed: %v", err)
			}
			if _, err := os.Stat(output); err != nil {
				t.Fatalf("expected output file to exist: %v", err)
			}
		})
	}
}

func TestAdvancedOperations(t *testing.T) {
	requireCGO(t)
	input := requireSampleDicom(t)
	outDir := t.TempDir()

	t.Run("ConvertImageRLE", func(t *testing.T) {
		output := filepath.Join(outDir, "convert-image-rle.dcm")
		err := ConvertImage(input, output, ConvertOptions{Compression: CompressionRLE})
		if err != nil {
			t.Fatalf("ConvertImage failed: %v", err)
		}
		if _, err := os.Stat(output); err != nil {
			t.Fatalf("expected output file to exist: %v", err)
		}
	})

	t.Run("ConvertDatasetTransferSyntax", func(t *testing.T) {
		output := filepath.Join(outDir, "transfer-syntax.dcm")
		if err := ConvertDatasetTransferSyntax(input, output, TransferSyntaxExplicitVRLittleEndian); err != nil {
			t.Fatalf("ConvertDatasetTransferSyntax failed: %v", err)
		}
		if _, err := os.Stat(output); err != nil {
			t.Fatalf("expected output file to exist: %v", err)
		}
	})

	t.Run("RemoveTags", func(t *testing.T) {
		output := filepath.Join(outDir, "removed-tags.dcm")
		opts := RemoveTagsOptions{RemovePrivate: true, RemoveRetired: true, RemoveGroupLength: true}
		if err := RemoveTags(input, output, opts); err != nil {
			t.Fatalf("RemoveTags failed: %v", err)
		}
		if _, err := os.Stat(output); err != nil {
			t.Fatalf("expected output file to exist: %v", err)
		}
	})
}
