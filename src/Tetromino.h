#pragma once

#include <SDL.h>
#include <array>
#include <cstddef>

namespace tetris {

struct Cell {
    int x{};
    int y{};
};

enum class TetrominoType : int {
    I = 0,
    O,
    T,
    S,
    Z,
    J,
    L,
    Count
};

using Shape = std::array<Cell, 4>;

struct TetrominoDefinition {
    std::array<Shape, 4> rotations{};
    SDL_Color color{};
};

const TetrominoDefinition& definition(TetrominoType type);
const Shape& shape(TetrominoType type, int rotation);
SDL_Color color(TetrominoType type);
std::size_t tetrominoCount();

} // namespace tetris
