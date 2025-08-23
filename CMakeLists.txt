cmake_minimum_required(VERSION 3.17)
project(RapidOcrOnnx)

add_definitions(-DUNICODE -D_UNICODE)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-Wall -g -O0")
else ()
    add_definitions("-Wall")
endif ()

set(BUILD_SHARED_LIBS false)

# OnnxRuntime (CPU)
include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime-static/OnnxRuntimeWrapper.cmake)
find_package(OnnxRuntime REQUIRED)
message(STATUS "OnnxRuntime_LIBS: ${OnnxRuntime_LIBS}")
message(STATUS "OnnxRuntime_INCLUDE_DIRS: ${OnnxRuntime_INCLUDE_DIRS}")

# OpenCV (static)
include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv-static/OpenCVWrapperConfig.cmake)
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")

# Windows Link CRT (optional)
if (OCR_BUILD_CRT STREQUAL "True")
    include(${CMAKE_CURRENT_SOURCE_DIR}/OcrCRTLinkage.cmake)
endif ()

# sources
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB THIRD_PARTY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/*/*.cpp)
add_library(RapidOcrOnnx SHARED ${OCR_SRC} ${THIRD_PARTY_SRC})
target_include_directories(RapidOcrOnnx PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/clipper
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
)
target_compile_definitions(RapidOcrOnnx PRIVATE OCRLIB_EXPORTS)
target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})

# Linux static link libstdc++
if (UNIX AND NOT APPLE)
    target_link_options(RapidOcrOnnx PRIVATE -static-libstdc++ -static-libgcc)
    target_link_libraries(RapidOcrOnnx dl pthread rt m)
endif ()

# install
install(TARGETS RapidOcrOnnx EXPORT RapidOcrOnnx)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "OcrLiteCApi.h")
