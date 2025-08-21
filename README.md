# RapidOcrOnnx - C Library

RapidOcrOnnx is a C library for fast and accurate OCR text detection and recognition using ONNX Runtime.

## Dependencies

Please refer to [third_party/README.md](third_party/README.md) for detailed download instructions and platform-specific setup.

The key dependencies (OpenCV and ONNX Runtime) need to be downloaded from the following sources:
- **OpenCV**: https://github.com/RapidAI/OpenCVBuilder/releases
- **ONNX Runtime**: https://github.com/RapidAI/OnnxruntimeBuilder/releases

## Building

### Prerequisites

- CMake 3.15+
- C++ compiler with C++17 support
- Platform-specific dependencies (see [third_party/README.md](third_party/README.md))

### Build Methods

#### Method 1: Native Build (Unix/Linux/macOS)

Use the provided `build.sh` script for native compilation:

```bash
./build.sh
```

#### Method 2: Portable Linux Build (Docker)

Use `build-linux-portable.sh` for cross-platform Linux builds using Docker:

```bash
./build-linux-portable.sh
```

### Build Output

After successful compilation, the library and headers will be installed to:
- **Default**: `build/install/`
- **Custom**: Specified by `INSTALL_PREFIX` environment variable

The installation directory will contain:
```
install/
├── include/     # Header files
├── lib/         # Static libraries
└── bin/         # Executables (if any)
```