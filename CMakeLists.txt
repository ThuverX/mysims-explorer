cmake_minimum_required(VERSION 3.18)
project(MSE VERSION 0.1.0 LANGUAGES CXX C)

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

set(IMGUI_SOURCES
    # ImGui
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    # ImGui SDL3 + OpenGL 3.3
    lib/imgui/backends/imgui_impl_sdl3.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

set(NFD_SOURCES
    lib/nfd/src/nfd_common.c
    lib/nfd/src/nfd_win.cpp
)

file(GLOB MSE_SOURCES
    source/*.hpp
    source/*.cpp
    source/gfx/*.hpp
    source/gfx/*.cpp
    source/io/*.cpp
    source/io/*.hpp
    source/util/*.cpp
    source/util/*.hpp
    source/ui/*.cpp
    source/ui/*.hpp
)

add_executable(mysims-explorer
    ${MSE_SOURCES}
    ${IMGUI_SOURCES}
    ${NFD_SOURCES}
    # GLAD
    lib/glad/src/gl.c)

target_include_directories(mysims-explorer SYSTEM PRIVATE
    lib/sdl/include
    lib/glad/include
    lib/glm
    lib/gli
    lib/imgui
    lib/nfd/src/include
    lib/tinyxml)
target_include_directories(mysims-explorer PRIVATE
    source
    lib/essencio/include
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mysims-explorer PRIVATE SDL3-static tinyxml2)

# Enable all warnings
if(MSVC)
    target_compile_options(mysims-explorer PRIVATE /W4 /permissive- /Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mysims-explorer PRIVATE -Wall -Wextra -Wpedantic)
endif()
