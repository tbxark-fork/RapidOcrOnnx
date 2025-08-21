cmake_minimum_required(VERSION 3.17)
project(RapidOcrOnnx)

add_definitions(-DUNICODE -D_UNICODE)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-Wall -g -O0")
else ()
    add_definitions("-Wall")
endif ()

# OnnxRuntime (CPU)
include(${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-static/OnnxRuntimeWrapper.cmake)
find_package(OnnxRuntime REQUIRED)
message(STATUS "OnnxRuntime_LIBS: ${OnnxRuntime_LIBS}")
message(STATUS "OnnxRuntime_INCLUDE_DIRS: ${OnnxRuntime_INCLUDE_DIRS}")

# OpenCV (static)
set(BUILD_SHARED_LIBS false)
include(${CMAKE_CURRENT_SOURCE_DIR}/opencv-static/OpenCVWrapperConfig.cmake)
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")

# sources
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(RapidOcrOnnx SHARED ${OCR_SRC})
target_include_directories(RapidOcrOnnx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(RapidOcrOnnx PRIVATE OCRLIB_EXPORTS)
target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})

# install
install(TARGETS RapidOcrOnnx EXPORT RapidOcrOnnx)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Windows Link CRT (optional)
if (OCR_BUILD_CRT STREQUAL "True")
    include(${CMAKE_CURRENT_SOURCE_DIR}/OcrCRTLinkage.cmake)
endif ()