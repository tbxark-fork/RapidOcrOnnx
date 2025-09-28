#!/usr/bin/env bash
# Build script for OCR server executable
# This script demonstrates how to build the OCR server executable

set -euo pipefail

echo "==> Building RapidOcrOnnx with server executable"

# Enable server support and server executable
export OCR_ENABLE_SERVER=ON
export OCR_BUILD_SERVER_EXECUTABLE=ON

# Run the build
./build.sh

echo "==> Server executable built successfully"
echo "    Executable location: build/install/bin/ocr-server"
echo ""
echo "Usage example:"
echo "    ./build/install/bin/ocr-server --help"
echo "    ./build/install/bin/ocr-server --port 8080 --det models/det.onnx --cls models/cls.onnx --rec models/rec.onnx --keys keys.txt"