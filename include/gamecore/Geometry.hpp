#pragma once

namespace gamecore {

struct Vec2i final {
    int x{};
    int y{};

    [[nodiscard]] constexpr auto operator+(const Vec2i other) const noexcept -> Vec2i {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr auto operator-(const Vec2i other) const noexcept -> Vec2i {
        return {x - other.x, y - other.y};
    }
};

struct Recti final {
    int x{};
    int y{};
    int width{};
    int height{};
};

} // namespace gamecore
