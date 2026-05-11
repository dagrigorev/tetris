#pragma once

#include <SDL.h>

#include <array>
#include <cstddef>

namespace tetris {

struct Cell final {
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

struct TetrominoDefinition final {
    std::array<Shape, 4> rotations{};
    SDL_Color color{};
};

[[nodiscard]] const TetrominoDefinition& definition(TetrominoType type);
[[nodiscard]] const Shape& shape(TetrominoType type, int rotation);
[[nodiscard]] SDL_Color color(TetrominoType type);
[[nodiscard]] std::size_t tetrominoCount() noexcept;

} // namespace tetris
