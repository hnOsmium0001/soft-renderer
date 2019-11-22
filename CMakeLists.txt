cmake_minimum_required(VERSION 3.0)
SET(CMAKE_C_COMPILER /usr/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)

project(SoftRenderer)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_subdirectory(external/custom/)
include_directories(
  external/Eigen
)

add_executable(soft_renderer
  src/main.cpp
  src/render.hpp
  src/render.cpp
)
target_link_libraries(soft_renderer external_libraries)
