#pragma once

#include <array>
#include <cstdint>

#include "gamecore/Color.hpp"
#include "gamecore/Geometry.hpp"

namespace games::tetris {

enum class TetrominoKind : std::uint8_t {
    I,
    O,
    T,
    S,
    Z,
    J,
    L,
    Count
};

using ShapeCells = std::array<gamecore::Vec2i, 4>;

struct Tetromino final {
    TetrominoKind kind{TetrominoKind::I};
    gamecore::Vec2i position{3, 0};
    int rotation{};

    [[nodiscard]] auto blocks() const -> ShapeCells;
    [[nodiscard]] auto color() const -> gamecore::Color;
    [[nodiscard]] auto rotated(int direction) const -> Tetromino;
};

[[nodiscard]] auto colorOf(TetrominoKind kind) -> gamecore::Color;
[[nodiscard]] auto spawnPosition(TetrominoKind kind) -> gamecore::Vec2i;

} // namespace games::tetris
