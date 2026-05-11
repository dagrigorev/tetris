#include "games/tetris/Tetromino.hpp"

#include <array>
#include <utility>

namespace games::tetris {
namespace {

using gamecore::Vec2i;

constexpr auto BaseShapes = std::array<ShapeCells, 7>{
    ShapeCells{Vec2i{0, 1}, Vec2i{1, 1}, Vec2i{2, 1}, Vec2i{3, 1}}, // I
    ShapeCells{Vec2i{1, 0}, Vec2i{2, 0}, Vec2i{1, 1}, Vec2i{2, 1}}, // O
    ShapeCells{Vec2i{1, 0}, Vec2i{0, 1}, Vec2i{1, 1}, Vec2i{2, 1}}, // T
    ShapeCells{Vec2i{1, 0}, Vec2i{2, 0}, Vec2i{0, 1}, Vec2i{1, 1}}, // S
    ShapeCells{Vec2i{0, 0}, Vec2i{1, 0}, Vec2i{1, 1}, Vec2i{2, 1}}, // Z
    ShapeCells{Vec2i{0, 0}, Vec2i{0, 1}, Vec2i{1, 1}, Vec2i{2, 1}}, // J
    ShapeCells{Vec2i{2, 0}, Vec2i{0, 1}, Vec2i{1, 1}, Vec2i{2, 1}}  // L
};

[[nodiscard]] constexpr auto kindIndex(const TetrominoKind kind) noexcept -> std::size_t {
    return static_cast<std::size_t>(std::to_underlying(kind));
}

[[nodiscard]] constexpr auto rotateCell(Vec2i p) noexcept -> Vec2i {
    // Rotate within a 4x4 box around center (1.5, 1.5): (x, y) -> (3 - y, x)
    return {3 - p.y, p.x};
}

} // namespace

auto Tetromino::blocks() const -> ShapeCells {
    auto result = BaseShapes[kindIndex(kind)];
    const auto turns = ((rotation % 4) + 4) % 4;

    if (kind == TetrominoKind::O) {
        for (auto& p : result) {
            p = p + position;
        }
        return result;
    }

    for (int i = 0; i < turns; ++i) {
        for (auto& p : result) {
            p = rotateCell(p);
        }
    }

    for (auto& p : result) {
        p = p + position;
    }

    return result;
}

auto Tetromino::color() const -> gamecore::Color {
    return colorOf(kind);
}

auto Tetromino::rotated(const int direction) const -> Tetromino {
    auto copy = *this;
    copy.rotation += direction;
    return copy;
}

auto colorOf(const TetrominoKind kind) -> gamecore::Color {
    using enum TetrominoKind;
    switch (kind) {
        case I: return {0, 240, 240, 255};
        case O: return {240, 240, 0, 255};
        case T: return {160, 0, 240, 255};
        case S: return {0, 220, 80, 255};
        case Z: return {240, 40, 40, 255};
        case J: return {40, 80, 240, 255};
        case L: return {240, 160, 0, 255};
        case Count: return gamecore::White;
    }
    return gamecore::White;
}

auto spawnPosition(const TetrominoKind kind) -> gamecore::Vec2i {
    if (kind == TetrominoKind::I) {
        return {3, -1};
    }
    return {3, 0};
}

} // namespace games::tetris
