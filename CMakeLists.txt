cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Tetris VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the compiler (optional, CMake should detect it)
set(CMAKE_CXX_COMPILER g++)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find SDL2 library
set(SDL2_INCLUDE_DIR "/usr/local/include")
set(SDL2_LIBRARY_DIR "/usr/local/lib")
set(SDL2_DLL_DIR "/usr/local/bin")

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIBRARY_DIR})

# Add executable
add_executable(tetris game.cpp)

# Link SDL2 libraries
target_link_libraries(tetris mingw32 SDL2main SDL2)

# Post-build step to copy SDL2 DLLs to output directory
add_custom_command(TARGET tetris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DLL_DIR}/SDL2.dll" $<TARGET_FILE_DIR:tetris>)
