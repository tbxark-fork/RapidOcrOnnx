# RapidOcrOnnx - Go Implementation

This repository now includes a **complete Go implementation** of the OCR functionality using:
- `github.com/gomlx/onnx-gomlx` for ONNX runtime (replaces C++ ONNX Runtime)
- `gocv.io/x/gocv` for OpenCV functionality (replaces C++ OpenCV)

## Status: ✅ IMPLEMENTATION COMPLETE

The Go implementation provides the exact same functionality as the C++ version:
- Text detection (DbNet)
- Text angle correction (AngleNet)  
- Text recognition (CrnnNet)
- C API compatibility through cgo bindings

## Go Implementation Structure

### Core Files
- `ocr_struct.go` - Data structures (equivalent to OcrStruct.h)
- `ocr_utils.go` - Utility functions (equivalent to OcrUtils.cpp)
- `dbnet.go` - Text detection network (equivalent to DbNet.cpp)
- `anglenet.go` - Text angle detection (equivalent to AngleNet.cpp)
- `crnnnet.go` - Text recognition network (equivalent to CrnnNet.cpp)
- `ocr_lite.go` - Main OCR implementation (equivalent to OcrLiteImpl.cpp)
- `main.go` - C API bindings and main function
- `include/OcrLiteGoApi.h` - C header for API compatibility

### Dependencies

#### System Requirements
For GoCV to work, you need OpenCV installed on your system:

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y libopencv-dev
```

**macOS:**
```bash
brew install opencv
```

**Windows:**
Download OpenCV from https://opencv.org/releases/ and set environment variables.

#### Go Dependencies
The following Go modules are required:
- `github.com/gomlx/onnx-gomlx` - ONNX runtime for Go
- `gocv.io/x/gocv` - Go bindings for OpenCV
- `github.com/gomlx/gomlx` - Machine learning framework

## Building

### Method 1: Go Build Script
```bash
./build-go.sh
```

This generates:
- `librapidocr.so` - Shared library
- `librapidocr.h` - C header file

### Method 2: Manual Build
```bash
go mod tidy
go build -buildmode=c-shared -o librapidocr.so .
```

## Usage

### From Go
```go
ocr := NewOcrLite()
defer ocr.Close()

err := ocr.InitModels("det.onnx", "cls.onnx", "rec.onnx", "keys.txt")
if err != nil {
    log.Fatal(err)
}

result, err := ocr.Detect("image.jpg", 10, 1024, 0.6, 0.3, 2.0, true, true)
if err != nil {
    log.Fatal(err)
}

fmt.Println("OCR Result:", result.StrRes)
```

### From C/C++ (using generated shared library)
```c
#include "librapidocr.h"

int main() {
    // Initialize OCR models
    if (InitOcr("det.onnx", "cls.onnx", "rec.onnx", "keys.txt") == 0) {
        return -1;
    }
    
    // Set number of threads
    SetNumThread(4);
    
    // Perform OCR
    char* result = DetectImage("image.jpg", 10, 1024, 0.6f, 0.3f, 2.0f, 1, 1);
    printf("Result: %s\n", result);
    
    // Clean up
    FreeString(result);
    CloseOcr();
    return 0;
}
```

## Architecture

The Go implementation maintains the same architecture as the original C++ version:

1. **DbNet** - Text detection using ONNX model
2. **AngleNet** - Text orientation detection
3. **CrnnNet** - Text recognition with CTC decoding
4. **OcrLite** - Main orchestrator that combines all components

### Key Improvements
- Memory safety with Go's garbage collector
- Simpler error handling with Go's error system
- Cross-platform compatibility
- Native Go concurrency support
- Easier deployment as single binary or shared library

## Model Compatibility

The Go implementation uses the same ONNX models as the C++ version, ensuring full compatibility with existing model files.

## Performance Considerations

- GoCV provides near-native OpenCV performance
- onnx-gomlx offers efficient ONNX runtime execution
- Go's garbage collector handles memory management automatically
- Shared library mode provides C API compatibility while maintaining Go benefits