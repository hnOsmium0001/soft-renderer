set_languages("c++20")

add_requires("vcpkg::glfw3", {alias = "glfw3"})
add_requires("vcpkg::glm", {alias = "glm"})
add_requires("vcpkg::imgui[core,glfw-binding,opengl3-binding]", {alias = "imgui"})
add_requires("vcpkg::imguizmo", {alias = "imguizmo"})
add_requires("vcpkg::nativefiledialog[core,zenity]", {alias = "nativefiledialog"})

target("soft-renderer-v2")
    set_kind("binary")
    add_files("source/**.cpp")
    add_includedirs("source/")
    add_packages("glfw3", "glm", "imgui", "imguizmo", "nativefiledialog")
