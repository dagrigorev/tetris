cmake_minimum_required(VERSION 3.24)

project(LayeredGames VERSION 2.0.0 LANGUAGES CXX)

option(LAYERED_GAMES_FETCH_SDL2 "Fetch SDL2 with FetchContent" ON)
option(LAYERED_GAMES_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

if(LAYERED_GAMES_FETCH_SDL2)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.11
        GIT_SHALLOW TRUE
    )
    set(SDL_SHARED ON CACHE BOOL "Build SDL2 shared library" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Do not build SDL2 static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Disable SDL2 tests" FORCE)
    FetchContent_MakeAvailable(SDL2)
else()
    find_package(SDL2 CONFIG REQUIRED)
endif()

add_library(layered_games_options INTERFACE)
target_compile_features(layered_games_options INTERFACE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(layered_games_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    )
    if(LAYERED_GAMES_WARNINGS_AS_ERRORS)
        target_compile_options(layered_games_options INTERFACE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(layered_games_options INTERFACE /W4 /permissive-)
    if(LAYERED_GAMES_WARNINGS_AS_ERRORS)
        target_compile_options(layered_games_options INTERFACE /WX)
    endif()
endif()

add_library(game_core STATIC
    src/core/Application.cpp
    src/core/EventBus.cpp
    src/core/GameRegistry.cpp
    src/core/GameShell.cpp
)

target_include_directories(game_core PUBLIC include)
target_link_libraries(game_core PUBLIC layered_games_options)

add_library(game_tetris STATIC
    src/games/tetris/Board.cpp
    src/games/tetris/ClassicScoringStrategy.cpp
    src/games/tetris/SevenBagTetrominoProvider.cpp
    src/games/tetris/Tetromino.cpp
    src/games/tetris/TetrisGame.cpp
    src/games/tetris/TetrisGameFactory.cpp
)

target_include_directories(game_tetris PUBLIC include)
target_link_libraries(game_tetris PUBLIC game_core layered_games_options)

add_library(game_arkanoid STATIC
    src/games/arkanoid/ArkanoidGame.cpp
    src/games/arkanoid/ArkanoidGameFactory.cpp
)

target_include_directories(game_arkanoid PUBLIC include)
target_link_libraries(game_arkanoid PUBLIC game_core layered_games_options)

add_library(platform_sdl2 STATIC
    src/platform/sdl2/BitmapFont.cpp
    src/platform/sdl2/Sdl2InputSource.cpp
    src/platform/sdl2/Sdl2Renderer2D.cpp
    src/platform/sdl2/Sdl2Window.cpp
)

target_include_directories(platform_sdl2 PUBLIC include)
target_link_libraries(platform_sdl2 PUBLIC game_core layered_games_options SDL2::SDL2)
target_compile_definitions(platform_sdl2 PUBLIC SDL_MAIN_HANDLED)

add_executable(layered_games src/main.cpp)
target_link_libraries(layered_games PRIVATE game_core game_tetris game_arkanoid platform_sdl2 layered_games_options)
target_compile_definitions(layered_games PRIVATE SDL_MAIN_HANDLED)

if(WIN32)
    add_custom_command(TARGET layered_games POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:layered_games>
        VERBATIM
    )
endif()
