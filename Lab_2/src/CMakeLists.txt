cmake_minimum_required(VERSION 3.16)
project(Lab2)

SET(CMAKE_BUILD_TYPE "Release")
#SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET(BASE_DIR ${CMAKE_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/../bin)

# Find OpenCV using pkg-config0
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(OpenCV opencv4)
    if(NOT OpenCV_FOUND)
        pkg_check_modules(OpenCV opencv)
    endif()
endif()

# Image processing tools (only if OpenCV is found)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found - building image processing tools")
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
    add_compile_options(${OpenCV_CFLAGS_OTHER})
    
    # Image inverter
    add_executable(image_inverter image_inverter.cpp)
    target_link_libraries(image_inverter ${OpenCV_LIBRARIES})
    
    # Image light modifier
    add_executable(image_light_modifier image_ligth_modifier.cpp)
    target_link_libraries(image_light_modifier ${OpenCV_LIBRARIES})
    
    # Image mirror
    add_executable(image_mirror image_mirror.cpp)
    target_link_libraries(image_mirror ${OpenCV_LIBRARIES})
    
    # RGB splitter
    add_executable(image_rgb_splitter image_rgb_splitter.cpp)
    target_link_libraries(image_rgb_splitter ${OpenCV_LIBRARIES})
    
    # Image rotate
    add_executable(image_rotate image_rotate.cpp)
    target_link_libraries(image_rotate ${OpenCV_LIBRARIES})
else()
    message(WARNING "OpenCV not found - image processing tools will not be built")
    message(WARNING "Install OpenCV with: sudo apt-get install libopencv-dev")
    message(WARNING "Or ensure pkg-config can find opencv4: pkg-config --cflags --libs opencv4")
endif()

# BitStream library
add_library(Common OBJECT)
target_sources(Common PRIVATE 
    bit_stream/src/bit_stream.cpp 
    bit_stream/src/byte_stream.cpp
)
target_include_directories(Common PUBLIC ${CMAKE_SOURCE_DIR})

# Golomb encoding/decoding
add_library(GolombLib OBJECT)
target_sources(GolombLib PRIVATE GolombUtils.cpp)
target_include_directories(GolombLib PUBLIC ${CMAKE_SOURCE_DIR})

# Golomb main executable
add_executable(golomb golomb_main.cpp $<TARGET_OBJECTS:GolombLib> $<TARGET_OBJECTS:Common>)

