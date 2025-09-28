#!/usr/bin/env bash
set -euo pipefail

echo "==> Building RapidOcrOnnx Go implementation"

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Error: Go is not installed"
    exit 1
fi

# Check if required system libraries are available for gocv
echo "Checking system dependencies..."

# Build the Go shared library
echo "Building Go shared library..."
go build -buildmode=c-shared -o librapidocr.so .

if [ $? -eq 0 ]; then
    echo "==> Build successful!"
    echo "Generated files:"
    echo "  - librapidocr.so (shared library)"
    echo "  - librapidocr.h (C header)"
    echo ""
    echo "Usage example in C/C++:"
    echo "  #include \"librapidocr.h\""
    echo "  InitOcr(\"det.onnx\", \"cls.onnx\", \"rec.onnx\", \"keys.txt\");"
    echo "  char* result = DetectImage(\"image.jpg\", 10, 1024, 0.6, 0.3, 2.0, 1, 1);"
    echo "  printf(\"%s\\n\", result);"
    echo "  FreeString(result);"
    echo "  CloseOcr();"
else
    echo "==> Build failed!"
    exit 1
fi