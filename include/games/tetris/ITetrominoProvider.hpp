#pragma once

#include "games/tetris/Tetromino.hpp"

namespace games::tetris {

class ITetrominoProvider {
public:
    virtual ~ITetrominoProvider() = default;
    [[nodiscard]] virtual auto next() -> TetrominoKind = 0;
};

} // namespace games::tetris
