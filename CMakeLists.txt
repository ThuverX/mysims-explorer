cmake_minimum_required(VERSION 3.18)
project(mysims-explorer VERSION 0.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_STATIC ON CACHE BOOL "")
set(SDL_SHARED OFF CACHE BOOL "")

add_subdirectory(lib/sdl)

add_executable(mysims-explorer source/main.cpp
    source/Context.hpp
    source/Context.cpp
    source/File.hpp
    source/File.cpp
    source/Renderer.hpp
    source/Renderer.cpp
    lib/glad/src/gl.c)

target_include_directories(mysims-explorer SYSTEM PRIVATE
    lib/sdl/include
    lib/glad/include
    lib/glm
    lib/gli
    lib/essencio/include)
target_link_libraries(mysims-explorer PRIVATE SDL3-static)

# Enable all warnings
if(MSVC)
    target_compile_options(mysims-explorer PRIVATE /W4 /permissive- /Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mysims-explorer PRIVATE -Wall -Wextra -Wpedantic)
endif()
