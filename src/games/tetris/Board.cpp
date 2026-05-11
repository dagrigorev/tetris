#include "games/tetris/Board.hpp"

#include <algorithm>

namespace games::tetris {

void Board::clear() noexcept {
    cells_.fill(0);
}

auto Board::isInside(const gamecore::Vec2i p) const noexcept -> bool {
    return p.x >= 0 && p.x < BoardWidth && p.y >= 0 && p.y < BoardHeight;
}

auto Board::isOccupied(const gamecore::Vec2i p) const noexcept -> bool {
    return isInside(p) && cells_[index(p)] != 0;
}

auto Board::cell(const gamecore::Vec2i p) const noexcept -> int {
    if (!isInside(p)) {
        return 0;
    }
    return cells_[index(p)];
}

void Board::setCell(const gamecore::Vec2i p, const int value) noexcept {
    if (isInside(p)) {
        cells_[index(p)] = value;
    }
}

auto Board::clearFullLines() noexcept -> int {
    int writeY = BoardHeight - 1;
    int cleared = 0;

    for (int readY = BoardHeight - 1; readY >= 0; --readY) {
        bool full = true;
        for (int x = 0; x < BoardWidth; ++x) {
            if (cell({x, readY}) == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            ++cleared;
            continue;
        }

        if (writeY != readY) {
            for (int x = 0; x < BoardWidth; ++x) {
                setCell({x, writeY}, cell({x, readY}));
            }
        }
        --writeY;
    }

    for (int y = writeY; y >= 0; --y) {
        for (int x = 0; x < BoardWidth; ++x) {
            setCell({x, y}, 0);
        }
    }

    return cleared;
}

} // namespace games::tetris
