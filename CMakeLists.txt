cmake_minimum_required(VERSION 3.23)
project(parallel_grep)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

# Add MPI Package to Project
find_package(MPI REQUIRED)

add_executable(parallel_grep
        grep-main.cpp
        grep.cpp
        grep.h)

# Add libraries for code completion and compiling
target_link_libraries(parallel_grep PUBLIC MPI::MPI_CXX)
