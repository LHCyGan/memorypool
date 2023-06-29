cmake_minimum_required(VERSION 3.16)
project(memorypool)

set(CMAKE_CXX_STANDARD 11)

add_executable(memorypool test.cpp include/common.h include/objectpool.h)
