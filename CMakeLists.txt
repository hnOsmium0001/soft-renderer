cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(SoftRenderer)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  external
)

add_executable(soft_renderer
  src/model.hpp
  src/model.cpp
  src/tgaimage.hpp
  src/tgaimage.cpp

  src/main.cpp
  src/render.hpp
  src/render.cpp
  src/old.hpp
  src/old.cpp
)
target_link_libraries(soft_renderer)
