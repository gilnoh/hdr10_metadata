cmake_minimum_required(VERSION 3.6)
project(hdr10_metadata)

set(CMAKE_CXX_STANDARD 11)

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp PQTable.cpp PQTable.h)
add_executable(hdr10_metadata ${SOURCE_FILES})