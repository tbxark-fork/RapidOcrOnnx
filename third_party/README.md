# Third Party Dependencies

This directory contains third-party libraries used by RapidOcrOnnx.

## Clipper
- **Version**: 6.4.2
- **Author**: Angus Johnson
- **License**: Boost Software License Ver 1.0
- **Website**: http://www.angusj.com
- **Description**: A polygon clipping and offsetting library
- **Files**: 
  - `clipper/clipper.hpp`
  - `clipper/clipper.cpp`

## nlohmann/json
- **Version**: 3.12.0
- **Author**: Niels Lohmann
- **License**: MIT
- **Website**: https://github.com/nlohmann/json
- **Description**: JSON for Modern C++
- **Files**: 
  - `nlohmann/json.hpp`

## OpenCV (Static)
- **Version**: 4.x
- **License**: Apache 2.0
- **Website**: https://opencv.org
- **Description**: Open Source Computer Vision Library (static build)
- **Directory**: `opencv-static/`
- **Contents**:
  - Static libraries (libopencv_*.a)
  - Headers (include/opencv4/)
  - CMake configuration files
  - Third-party libraries (JPEG, PNG, TIFF, etc.)

## ONNX Runtime (Static)
- **License**: MIT
- **Website**: https://onnxruntime.ai
- **Description**: Cross-platform machine learning model accelerator (static build)
- **Directory**: `onnxruntime-static/`
- **Contents**:
  - Static library (libonnxruntime.a)
  - C/C++ API headers
  - CMake configuration files

## Usage

These libraries are automatically included in the build process through CMakeLists.txt. The include directories are:
- `third_party/clipper` for clipper.hpp
- `third_party/nlohmann` for json.hpp
- OpenCV and ONNX Runtime are configured through their respective CMake wrapper files

No changes to existing source code are required as the include paths are configured in CMakeLists.txt.
