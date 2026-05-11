#include "Tetromino.h"

#include <array>
#include <stdexcept>

namespace tetris {
namespace {

constexpr Cell c(int x, int y) noexcept { return Cell{x, y}; }

const std::array<TetrominoDefinition, static_cast<std::size_t>(TetrominoType::Count)> kDefinitions = {
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(0, 1), c(1, 1), c(2, 1), c(3, 1)},
            Shape{c(2, 0), c(2, 1), c(2, 2), c(2, 3)},
            Shape{c(0, 2), c(1, 2), c(2, 2), c(3, 2)},
            Shape{c(1, 0), c(1, 1), c(1, 2), c(1, 3)}
        },
        SDL_Color{0, 220, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(1, 0), c(2, 0), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(2, 0), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(2, 0), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(2, 0), c(1, 1), c(2, 1)}
        },
        SDL_Color{255, 220, 0, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(1, 0), c(0, 1), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(1, 1), c(2, 1), c(1, 2)},
            Shape{c(0, 1), c(1, 1), c(2, 1), c(1, 2)},
            Shape{c(1, 0), c(0, 1), c(1, 1), c(1, 2)}
        },
        SDL_Color{180, 90, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(1, 0), c(2, 0), c(0, 1), c(1, 1)},
            Shape{c(1, 0), c(1, 1), c(2, 1), c(2, 2)},
            Shape{c(1, 1), c(2, 1), c(0, 2), c(1, 2)},
            Shape{c(0, 0), c(0, 1), c(1, 1), c(1, 2)}
        },
        SDL_Color{90, 230, 80, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(0, 0), c(1, 0), c(1, 1), c(2, 1)},
            Shape{c(2, 0), c(1, 1), c(2, 1), c(1, 2)},
            Shape{c(0, 1), c(1, 1), c(1, 2), c(2, 2)},
            Shape{c(1, 0), c(0, 1), c(1, 1), c(0, 2)}
        },
        SDL_Color{255, 80, 95, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(0, 0), c(0, 1), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(2, 0), c(1, 1), c(1, 2)},
            Shape{c(0, 1), c(1, 1), c(2, 1), c(2, 2)},
            Shape{c(1, 0), c(1, 1), c(0, 2), c(1, 2)}
        },
        SDL_Color{70, 130, 255, 255}
    },
    TetrominoDefinition{
        std::array<Shape, 4>{
            Shape{c(2, 0), c(0, 1), c(1, 1), c(2, 1)},
            Shape{c(1, 0), c(1, 1), c(1, 2), c(2, 2)},
            Shape{c(0, 1), c(1, 1), c(2, 1), c(0, 2)},
            Shape{c(0, 0), c(1, 0), c(1, 1), c(1, 2)}
        },
        SDL_Color{255, 160, 40, 255}
    }
};

std::size_t indexOf(TetrominoType type) {
    const auto index = static_cast<std::size_t>(type);
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
    const auto normalizedRotation = ((rotation % 4) + 4) % 4;
    return definition(type).rotations[static_cast<std::size_t>(normalizedRotation)];
}

SDL_Color color(TetrominoType type) {
    return definition(type).color;
}

std::size_t tetrominoCount() {
    return kDefinitions.size();
}

} // namespace tetris
