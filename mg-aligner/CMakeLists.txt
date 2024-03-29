cmake_minimum_required(VERSION 3.10)
project(mg-aligner)

# THIS HAS TO COME BEFORE THE PROJECT LINE
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# find_package(OPENMP REQUIRED) # Find the package

set(CMAKE_CXX_STANDARD 17)

SET_SOURCE_FILES_PROPERTIES(bwt.c align.c exact_match.c inexact_match.c is.c main.c io.c PROPERTIES LANGUAGE CXX)

add_executable(bwbble bwt.c bwt.h align.c align.h exact_match.c exact_match.h
        inexact_match.c inexact_match.h is.c main.c io.c io.h common.h)

target_link_libraries(bwbble Threads::Threads)
target_link_libraries(bwbble m)
target_link_libraries(bwbble z)
# target_link_libraries(bwbble ${OpenMP_CXX_LIBRARIES}) # Link against it for C++
