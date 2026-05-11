#pragma once

#include <cstdint>

namespace gamecore {

struct Color final {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t a{255};
};

inline constexpr Color Black{0, 0, 0, 255};
inline constexpr Color White{255, 255, 255, 255};
inline constexpr Color DarkGray{24, 24, 28, 255};
inline constexpr Color MidGray{64, 64, 72, 255};
inline constexpr Color LightGray{128, 128, 140, 255};

} // namespace gamecore
