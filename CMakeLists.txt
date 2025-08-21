cmake_minimum_required(VERSION 3.17)
project(RapidOcrOnnx)

# Build only the shared C library (CLIB), CPU-only.

add_definitions(-DUNICODE -D_UNICODE)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-Wall -g -O0")
else ()
    add_definitions("-Wall")
endif ()

# OnnxRuntime (CPU)
include(${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-static/OnnxRuntimeWrapper.cmake)
find_package(OnnxRuntime REQUIRED)

# OpenCV (static)
set(BUILD_SHARED_LIBS false)
include(${CMAKE_CURRENT_SOURCE_DIR}/opencv-static/OpenCVWrapperConfig.cmake)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

# sources
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(OCR_COMPILE_CODE ${OCR_SRC})

add_library(RapidOcrOnnx SHARED ${OCR_COMPILE_CODE})
target_include_directories(RapidOcrOnnx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(RapidOcrOnnx PRIVATE OCRLIB_EXPORTS)
target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})



install(TARGETS RapidOcrOnnx EXPORT RapidOcrOnnx)
file(GLOB OCR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
install(FILES ${OCR_INCLUDE} DESTINATION include)

# Windows Link CRT (optional)
if (OCR_BUILD_CRT STREQUAL "True")
    include(${CMAKE_CURRENT_SOURCE_DIR}/OcrCRTLinkage.cmake)
endif ()
