# Find TensorRT library and include paths
# This module defines:
# TENSORRT_INCLUDE_DIRS - TensorRT include directories
# TENSORRT_LIBRARIES - TensorRT libraries
# TENSORRT_FOUND - True if TensorRT was found

# Try to find TensorRT in standard locations
find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    PATHS
        ${PACKMAN_DIR}/TensorRT/include
        ${PACKMAN_DIR}/tensorrt/include
        $ENV{TENSORRT_PATH}/include
        $ENV{CUDA_PATH}/include
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v*/include"
        /usr/include/x86_64-linux-gnu
        /usr/local/include
        /usr/include
    PATH_SUFFIXES include
)

# Find the TensorRT libraries
find_library(TENSORRT_LIBRARY
    NAMES nvinfer nvinfer_10.lib
    PATHS
        ${PACKMAN_DIR}/TensorRT/lib
        ${PACKMAN_DIR}/tensorrt/lib
        ${PACKMAN_DIR}/TensorRT/lib/x64
        ${PACKMAN_DIR}/tensorrt/lib/x64
        $ENV{TENSORRT_PATH}/lib
        $ENV{CUDA_PATH}/lib/x64
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v*/lib/x64"
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib lib64
)

find_library(TENSORRT_PARSER_LIBRARY
    NAMES nvonnxparser nvonnxparser_10.lib
    PATHS
        ${PACKMAN_DIR}/TensorRT/lib
        ${PACKMAN_DIR}/tensorrt/lib
        ${PACKMAN_DIR}/TensorRT/lib/x64
        ${PACKMAN_DIR}/tensorrt/lib/x64
        $ENV{TENSORRT_PATH}/lib
        $ENV{CUDA_PATH}/lib/x64
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v*/lib/x64"
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib lib64
)

# Print debug information

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TensorRT
    REQUIRED_VARS
        TENSORRT_INCLUDE_DIR
        TENSORRT_LIBRARY
        TENSORRT_PARSER_LIBRARY
)

if(TENSORRT_FOUND)
    set(TENSORRT_INCLUDE_DIRS ${TENSORRT_INCLUDE_DIR})
    set(TENSORRT_LIBRARIES ${TENSORRT_LIBRARY} ${TENSORRT_PARSER_LIBRARY})
endif()

mark_as_advanced(
    TENSORRT_INCLUDE_DIR
    TENSORRT_LIBRARY
    TENSORRT_PARSER_LIBRARY
)
