cmake_minimum_required(VERSION 3.21)
project(ParallelCanopyAlgorithm)

set(CMAKE_CXX_STANDARD 23)

# Add MPI Package to Project
find_package(MPI REQUIRED)

add_executable(ParallelCanopyAlgorithm main.cpp)

# Add libraries for code completion and compiling
target_link_libraries(ParallelCanopyAlgorithm PUBLIC MPI::MPI_CXX)

