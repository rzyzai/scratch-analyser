cmake_minimum_required(VERSION 3.24)
project(scratch_analyser)

set(CMAKE_CXX_STANDARD 17)

include_directories(include/bundled include)
add_executable(scratch_analyser src/main.cpp)
