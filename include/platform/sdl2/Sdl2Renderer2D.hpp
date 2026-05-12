#pragma once

#include <SDL2/SDL.h>

#include "gamecore/Renderer2D.hpp"

namespace platform::sdl2 {

class Sdl2Renderer2D final : public gamecore::IRenderer2D {
public:
    explicit Sdl2Renderer2D(SDL_Renderer* renderer);

    void beginFrame(gamecore::Color clearColor) override;
    void endFrame() override;
    void fillRect(gamecore::Recti rect, gamecore::Color color) override;
    void drawRect(gamecore::Recti rect, gamecore::Color color) override;
    void drawLine(gamecore::Vec2i from, gamecore::Vec2i to, gamecore::Color color) override;
    void fillCircle(gamecore::Vec2i center, int radius, gamecore::Color color) override;
    void drawCircle(gamecore::Vec2i center, int radius, gamecore::Color color) override;
    void fillTriangle(gamecore::Vec2i a, gamecore::Vec2i b, gamecore::Vec2i c, gamecore::Color color) override;
    void drawText(gamecore::Vec2i position, std::string_view text, gamecore::Color color, int scale) override;

private:
    SDL_Renderer* renderer_{};

    void setColor(gamecore::Color color);
};

} // namespace platform::sdl2
