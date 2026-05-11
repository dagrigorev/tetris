#pragma once

#include <array>
#include <optional>

#include "gamecore/Geometry.hpp"

namespace games::tetris {

inline constexpr int BoardWidth = 10;
inline constexpr int BoardHeight = 20;

class Board final {
public:
    using Cells = std::array<int, BoardWidth * BoardHeight>;

    void clear() noexcept;

    [[nodiscard]] auto isInside(gamecore::Vec2i p) const noexcept -> bool;
    [[nodiscard]] auto isOccupied(gamecore::Vec2i p) const noexcept -> bool;
    [[nodiscard]] auto cell(gamecore::Vec2i p) const noexcept -> int;

    void setCell(gamecore::Vec2i p, int value) noexcept;
    [[nodiscard]] auto clearFullLines() noexcept -> int;

    [[nodiscard]] auto cells() const noexcept -> const Cells& { return cells_; }

private:
    Cells cells_{};

    [[nodiscard]] static constexpr auto index(gamecore::Vec2i p) noexcept -> std::size_t {
        return static_cast<std::size_t>(p.y * BoardWidth + p.x);
    }
};

} // namespace games::tetris
