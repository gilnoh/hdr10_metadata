cmake_minimum_required(VERSION 3.6)
project(hdr10_metadata)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp PQTable.cpp PQTable.h dpx/dpx.cpp)
add_executable(hdr10_metadata ${SOURCE_FILES})

include_directories(dpx/)

#find_package(ImageMagick COMPONENTS Magick++)
#include_directories(${ImageMagick_INCLUDE_DIRS})
#target_link_libraries(hdr10_metadata ${ImageMagick_LIBRARIES})

