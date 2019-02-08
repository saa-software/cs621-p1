cmake_minimum_required(VERSION 3.13)
project(cs621-p1)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp lzw_compression.h point-to-point-net-device.h)

add_executable(cs621-p1 ${SOURCE_FILES})