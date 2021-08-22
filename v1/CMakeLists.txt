cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(SoftRenderer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(FORCE_COLORED_OUTPUT)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

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
