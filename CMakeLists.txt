cmake_minimum_required(VERSION 3.24)

project(Tetris
    VERSION 2.0.0
    DESCRIPTION "Clean SDL2 Tetris implementation"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# SDL2 is downloaded and built automatically by CMake.
# A fixed release tag keeps builds reproducible.
set(SDL_SHARED OFF CACHE BOOL "Build SDL2 shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL2 static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build SDL2 tests" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL2 test library" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "Install SDL2" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.10
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL2)

add_executable(tetris
    src/main.cpp
    src/App.cpp
    src/Game.cpp
    src/Renderer.cpp
    src/Tetromino.cpp
)

target_include_directories(tetris PRIVATE src)
target_compile_definitions(tetris PRIVATE SDL_MAIN_HANDLED)

if(TARGET SDL2::SDL2-static)
    target_link_libraries(tetris PRIVATE SDL2::SDL2-static)
elseif(TARGET SDL2::SDL2)
    target_link_libraries(tetris PRIVATE SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 target was not created by FetchContent")
endif()

if(MSVC)
    target_compile_options(tetris PRIVATE /W4 /permissive-)
else()
    target_compile_options(tetris PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(tetris PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
