cmake_minimum_required(VERSION 3.17)
project(RapidOcrOnnx)

# Set C++ standard to C++17 to support onnxruntime features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Server support (optional)
option(OCR_ENABLE_SERVER "Enable OCR server functionality" OFF)
option(OCR_BUILD_SERVER_EXECUTABLE "Build OCR server executable" OFF)
if(OCR_ENABLE_SERVER)
    message(STATUS "OCR Server support enabled")
    # Find asio library using pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ASIO REQUIRED asio)
    message(STATUS "ASIO_INCLUDE_DIRS: ${ASIO_INCLUDE_DIRS}")
    message(STATUS "ASIO_CFLAGS: ${ASIO_CFLAGS}")
    add_definitions(-DOCR_ENABLE_SERVER)
else()
    message(STATUS "OCR Server support disabled")
endif()

# Windows Link CRT (optional)
if (OCR_BUILD_CRT STREQUAL "True")
    include(${CMAKE_CURRENT_SOURCE_DIR}/OcrCRTLinkage.cmake)
endif ()

# sources
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB THIRD_PARTY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/*/*.cpp)

# Always exclude OcrServerMain.cpp from the library as it's only for the executable
list(REMOVE_ITEM OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/OcrServerMain.cpp)

# Conditionally exclude OcrServer.cpp if server support is disabled
if(NOT OCR_ENABLE_SERVER)
    list(REMOVE_ITEM OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/OcrServer.cpp)
endif()

add_library(RapidOcrOnnx SHARED ${OCR_SRC} ${THIRD_PARTY_SRC})
target_include_directories(RapidOcrOnnx PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/clipper
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
)

# Conditionally add server-related includes and compile options
if(OCR_ENABLE_SERVER)
    target_include_directories(RapidOcrOnnx PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/crow
        ${ASIO_INCLUDE_DIRS}
    )
    target_compile_options(RapidOcrOnnx PRIVATE ${ASIO_CFLAGS})
endif()

target_compile_definitions(RapidOcrOnnx PRIVATE OCRLIB_EXPORTS)
target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})

# Linux static link libstdc++
if (UNIX AND NOT APPLE)
    target_link_options(RapidOcrOnnx PRIVATE -static-libstdc++ -static-libgcc)
    target_link_libraries(RapidOcrOnnx dl pthread rt m)
endif ()

# install
if(OCR_BUILD_SERVER_EXECUTABLE AND OCR_ENABLE_SERVER)
    message(STATUS "Building OCR server executable")
    add_executable(ocr-server ${CMAKE_CURRENT_SOURCE_DIR}/src/OcrServerMain.cpp)
    target_include_directories(ocr-server PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cxxopts
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/crow
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
        ${ASIO_INCLUDE_DIRS}
    )
    target_compile_options(ocr-server PRIVATE ${ASIO_CFLAGS})
    target_link_libraries(ocr-server RapidOcrOnnx)
    
    # Set RPATH for macOS
    if(APPLE)
        set_target_properties(ocr-server PROPERTIES
            INSTALL_RPATH "@loader_path/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
    
    # Install the executable
    install(TARGETS ocr-server DESTINATION bin)
endif()

# Always install the library and headers
install(TARGETS RapidOcrOnnx EXPORT RapidOcrOnnx DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.h")
