# Third Party Dependencies

This directory contains third-party libraries used by RapidOcrOnnx.

## Download Instructions

### OpenCV (Static)
- **Download**: https://github.com/RapidAI/OpenCVBuilder/releases
- **File**: `opencv-static-{version}-{platform}.7z`
- **Platforms**: 
  - `windows-x64` (Windows 64-bit)
  - `windows-x86` (Windows 32-bit)
  - `linux` (Linux x64)
  - `macos` (macOS)
- **Directory Structure**:
```
opencv-static/
├── OpenCVWrapperConfig.cmake
├── linux/
├── macos/
├── windows-x64/
└── windows-x86/
```

### ONNX Runtime (Static)
- **Download**: https://github.com/RapidAI/OnnxruntimeBuilder/releases
- **File**: `onnxruntime-static-{version}-{platform}.7z`
- **Platforms**: Same as OpenCV
- **Directory Structure**:
```
onnxruntime-static/
├── OnnxRuntimeWrapper.cmake
├── linux/
├── macos/
├── windows-x64/
└── windows-x86/
```

### Setup
1. Download both archives for your platform
2. Extract to project root directory
3. Verify directory structure matches above
4. Run build script (build.sh/build.bat)

## Included Libraries

### Clipper
- **Version**: 6.4.2
- **Author**: Angus Johnson
- **License**: Boost Software License Ver 1.0
- **Website**: http://www.angusj.com
- **Description**: A polygon clipping and offsetting library
- **Files**: 
  - `clipper/clipper.hpp`
  - `clipper/clipper.cpp`

### nlohmann/json
- **Version**: 3.12.0
- **Author**: Niels Lohmann
- **License**: MIT
- **Website**: https://github.com/nlohmann/json
- **Description**: JSON for Modern C++
- **Files**: 
  - `nlohmann/json.hpp`
