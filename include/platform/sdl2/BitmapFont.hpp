#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <SDL2/SDL.h>

#include "gamecore/Color.hpp"
#include "gamecore/Geometry.hpp"

namespace platform::sdl2 {

class BitmapFont final {
public:
    using Glyph = std::array<std::uint8_t, 7>;

    static void drawText(SDL_Renderer* renderer,
                         gamecore::Vec2i position,
                         std::string_view text,
                         gamecore::Color color,
                         int scale);

private:
    [[nodiscard]] static auto glyphFor(char c) -> Glyph;
};

} // namespace platform::sdl2
