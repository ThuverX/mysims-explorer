cmake_minimum_required(VERSION 3.18)
project(MYSIMSEXPLORER VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    @ONLY
)

set(SDL_STATIC ON CACHE BOOL "")
set(SDL_SHARED OFF CACHE BOOL "")

add_subdirectory(lib/sdl)
add_subdirectory(lib/tinyxml)

add_executable(mysims-explorer source/main.cpp
    source/AssetMap.hpp
    source/AssetMap.cpp
    source/Camera.hpp
    source/Camera.cpp
    source/Context.hpp
    source/Context.cpp
    source/File.hpp
    source/File.cpp
    source/Loader.hpp
    source/Loader.cpp
    source/Logger.hpp
    source/Logger.cpp
    source/Renderer.hpp
    source/Renderer.cpp
    source/UI.hpp
    source/UI.cpp
    # ImGui
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    # ImGui SDL3 + OpenGL 3.3
    lib/imgui/backends/imgui_impl_sdl3.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    # ImGui File Dialog
    lib/imguifiledialog/ImGuiFileDialog.cpp
    # GLAD
    lib/glad/src/gl.c)

target_include_directories(mysims-explorer SYSTEM PRIVATE
    lib/sdl/include
    lib/glad/include
    lib/glm
    lib/gli
    lib/imgui
    lib/imguifiledialog
    lib/tinyxml)
target_include_directories(mysims-explorer PRIVATE
    lib/essencio/include
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mysims-explorer PRIVATE SDL3-static tinyxml2)

# Enable all warnings
if(MSVC)
    target_compile_options(mysims-explorer PRIVATE /W4 /permissive- /Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mysims-explorer PRIVATE -Wall -Wextra -Wpedantic)
endif()
