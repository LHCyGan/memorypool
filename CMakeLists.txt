cmake_minimum_required(VERSION 3.16)
project(memorypool)

set(CMAKE_CXX_STANDARD 11)
file(GLOB_RECURSE SOURCES "include/*.h" )
add_executable(memorypool unitest.cpp ${SOURCES} benchmark.cpp)
