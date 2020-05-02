cmake_minimum_required(VERSION 3.10)

project(Mandelbrot)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(MandelbrotImage MandelbrotImage.cpp)
target_link_libraries(MandelbrotImage SDL2)

add_executable(MandelbrotASCII MandelbrotASCII.cpp)
target_link_libraries(MandelbrotASCII SDL2)