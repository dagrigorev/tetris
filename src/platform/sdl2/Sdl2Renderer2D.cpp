#include "platform/sdl2/Sdl2Renderer2D.hpp"

#include "platform/sdl2/BitmapFont.hpp"

namespace platform::sdl2 {

Sdl2Renderer2D::Sdl2Renderer2D(SDL_Renderer* renderer) : renderer_(renderer) {}

void Sdl2Renderer2D::beginFrame(const gamecore::Color clearColor) {
    setColor(clearColor);
    SDL_RenderClear(renderer_);
}

void Sdl2Renderer2D::endFrame() {
    SDL_RenderPresent(renderer_);
}

void Sdl2Renderer2D::fillRect(const gamecore::Recti rect, const gamecore::Color color) {
    setColor(color);
    const SDL_Rect native{rect.x, rect.y, rect.width, rect.height};
    SDL_RenderFillRect(renderer_, &native);
}

void Sdl2Renderer2D::drawRect(const gamecore::Recti rect, const gamecore::Color color) {
    setColor(color);
    const SDL_Rect native{rect.x, rect.y, rect.width, rect.height};
    SDL_RenderDrawRect(renderer_, &native);
}

void Sdl2Renderer2D::drawLine(const gamecore::Vec2i from, const gamecore::Vec2i to, const gamecore::Color color) {
    setColor(color);
    SDL_RenderDrawLine(renderer_, from.x, from.y, to.x, to.y);
}

void Sdl2Renderer2D::drawText(const gamecore::Vec2i position,
                              const std::string_view text,
                              const gamecore::Color color,
                              const int scale) {
    BitmapFont::drawText(renderer_, position, text, color, scale);
}

void Sdl2Renderer2D::setColor(const gamecore::Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
}

} // namespace platform::sdl2
