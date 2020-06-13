cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(SoftRenderer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(
	src
)
add_executable(soft_renderer
	src/Util.hpp
	src/Util.cpp
	src/Model.hpp
	src/Model.cpp
	src/TGAImage.hpp
	src/TGAImage.cpp
	src/Render.hpp
	src/Render.cpp
	src/Main.cpp
)
target_link_libraries(soft_renderer ${CONAN_LIBS})

file(COPY obj/ DESTINATION ./obj)
