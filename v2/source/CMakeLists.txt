set(SUBPROJECT_EXECUTABLE SoftRenderer)
add_executable(${SUBPROJECT_EXECUTABLE}
    main.cpp

    Color.hpp
    Macros.hpp
    Margin.hpp
    Math.hpp
    Rect.hpp
    ScopeGuard.hpp
    Size.hpp
)

target_include_directories(${SUBPROJECT_EXECUTABLE}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${SUBPROJECT_EXECUTABLE}
PRIVATE
    ${CONAN_LIBS}
)

add_subdirectory(Renderer)
add_subdirectory(Viewer)
