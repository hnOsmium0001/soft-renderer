cmake_minimum_required(VERSION 3.0)
project(SoftRenderer)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(external/)

add_executable(soft_renderer
  src/main.cpp
  src/render.hpp
  src/render.cpp
)
target_link_libraries(soft_renderer external_libraries)
