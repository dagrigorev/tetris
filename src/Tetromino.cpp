#include "Tetromino.h"

#include <array>
#include <stdexcept>
#include <utility>

namespace tetris {
namespace {

[[nodiscard]] constexpr Cell cell(int x, int y) noexcept { return Cell{x, y}; }

using enum TetrominoType;

const std::array<TetrominoDefinition, static_cast<std::size_t>(TetrominoType::Count)> kDefinitions = {
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(0, 1), cell(1, 1), cell(2, 1), cell(3, 1)},
            Shape{cell(2, 0), cell(2, 1), cell(2, 2), cell(2, 3)},
            Shape{cell(0, 2), cell(1, 2), cell(2, 2), cell(3, 2)},
            Shape{cell(1, 0), cell(1, 1), cell(1, 2), cell(1, 3)}
        },
        SDL_Color{0, 220, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(1, 0), cell(2, 0), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(2, 0), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(2, 0), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(2, 0), cell(1, 1), cell(2, 1)}
        },
        SDL_Color{255, 220, 0, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(1, 0), cell(0, 1), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(1, 1), cell(2, 1), cell(1, 2)},
            Shape{cell(0, 1), cell(1, 1), cell(2, 1), cell(1, 2)},
            Shape{cell(1, 0), cell(0, 1), cell(1, 1), cell(1, 2)}
        },
        SDL_Color{180, 90, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(1, 0), cell(2, 0), cell(0, 1), cell(1, 1)},
            Shape{cell(1, 0), cell(1, 1), cell(2, 1), cell(2, 2)},
            Shape{cell(1, 1), cell(2, 1), cell(0, 2), cell(1, 2)},
            Shape{cell(0, 0), cell(0, 1), cell(1, 1), cell(1, 2)}
        },
        SDL_Color{90, 230, 80, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(0, 0), cell(1, 0), cell(1, 1), cell(2, 1)},
            Shape{cell(2, 0), cell(1, 1), cell(2, 1), cell(1, 2)},
            Shape{cell(0, 1), cell(1, 1), cell(1, 2), cell(2, 2)},
            Shape{cell(1, 0), cell(0, 1), cell(1, 1), cell(0, 2)}
        },
        SDL_Color{255, 80, 95, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(0, 0), cell(0, 1), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(2, 0), cell(1, 1), cell(1, 2)},
            Shape{cell(0, 1), cell(1, 1), cell(2, 1), cell(2, 2)},
            Shape{cell(1, 0), cell(1, 1), cell(0, 2), cell(1, 2)}
        },
        SDL_Color{70, 130, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{cell(2, 0), cell(0, 1), cell(1, 1), cell(2, 1)},
            Shape{cell(1, 0), cell(1, 1), cell(1, 2), cell(2, 2)},
            Shape{cell(0, 1), cell(1, 1), cell(2, 1), cell(0, 2)},
            Shape{cell(0, 0), cell(1, 0), cell(1, 1), cell(1, 2)}
        },
        SDL_Color{255, 160, 40, 255}
    }
};

[[nodiscard]] std::size_t indexOf(TetrominoType type) {
    const auto index = static_cast<std::size_t>(std::to_underlying(type));
    if (index >= kDefinitions.size()) {
        throw std::out_of_range("Invalid tetromino type");
    }
    return index;
}

} // namespace

const TetrominoDefinition& definition(TetrominoType type) {
    return kDefinitions[indexOf(type)];
}

const Shape& shape(TetrominoType type, int rotation) {
    const auto normalizedRotation = static_cast<std::size_t>(((rotation % 4) + 4) % 4);
    return definition(type).rotations[normalizedRotation];
}

SDL_Color color(TetrominoType type) {
    return definition(type).color;
}

std::size_t tetrominoCount() noexcept {
    return kDefinitions.size();
}

} // namespace tetris
