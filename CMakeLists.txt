cmake_minimum_required(VERSION 3.15)
project(url_compressor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -O3" )

add_executable(url_compressor src/main.cpp)