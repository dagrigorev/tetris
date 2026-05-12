#include "platform/sdl2/Sdl2Renderer2D.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "platform/sdl2/BitmapFont.hpp"

namespace platform::sdl2 {
namespace {

[[nodiscard]] auto edgeFunction(const gamecore::Vec2i a, const gamecore::Vec2i b, const gamecore::Vec2i p) -> int {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

} // namespace

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

void Sdl2Renderer2D::fillCircle(const gamecore::Vec2i center, const int radius, const gamecore::Color color) {
    if (radius <= 0) {
        return;
    }

    setColor(color);
    for (int dy = -radius; dy <= radius; ++dy) {
        const auto span = static_cast<int>(std::sqrt(static_cast<double>(radius * radius - dy * dy)));
        SDL_RenderDrawLine(renderer_, center.x - span, center.y + dy, center.x + span, center.y + dy);
    }
}

void Sdl2Renderer2D::drawCircle(const gamecore::Vec2i center, const int radius, const gamecore::Color color) {
    if (radius <= 0) {
        return;
    }

    setColor(color);
    auto x = radius - 1;
    auto y = 0;
    auto dx = 1;
    auto dy = 1;
    auto err = dx - (radius << 1);

    while (x >= y) {
        SDL_RenderDrawPoint(renderer_, center.x + x, center.y + y);
        SDL_RenderDrawPoint(renderer_, center.x + y, center.y + x);
        SDL_RenderDrawPoint(renderer_, center.x - y, center.y + x);
        SDL_RenderDrawPoint(renderer_, center.x - x, center.y + y);
        SDL_RenderDrawPoint(renderer_, center.x - x, center.y - y);
        SDL_RenderDrawPoint(renderer_, center.x - y, center.y - x);
        SDL_RenderDrawPoint(renderer_, center.x + y, center.y - x);
        SDL_RenderDrawPoint(renderer_, center.x + x, center.y - y);

        if (err <= 0) {
            ++y;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            --x;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void Sdl2Renderer2D::fillTriangle(gamecore::Vec2i a,
                                  gamecore::Vec2i b,
                                  gamecore::Vec2i c,
                                  const gamecore::Color color) {
    setColor(color);

    const auto minX = std::min({a.x, b.x, c.x});
    const auto maxX = std::max({a.x, b.x, c.x});
    const auto minY = std::min({a.y, b.y, c.y});
    const auto maxY = std::max({a.y, b.y, c.y});

    const auto area = edgeFunction(a, b, c);
    if (area == 0) {
        return;
    }

    for (auto y = minY; y <= maxY; ++y) {
        auto spanStart = maxX + 1;
        auto spanEnd = minX - 1;
        for (auto x = minX; x <= maxX; ++x) {
            const gamecore::Vec2i p{x, y};
            const auto w0 = edgeFunction(b, c, p);
            const auto w1 = edgeFunction(c, a, p);
            const auto w2 = edgeFunction(a, b, p);
            const auto inside = area > 0 ? (w0 >= 0 && w1 >= 0 && w2 >= 0) : (w0 <= 0 && w1 <= 0 && w2 <= 0);
            if (inside) {
                spanStart = std::min(spanStart, x);
                spanEnd = std::max(spanEnd, x);
            }
        }
        if (spanStart <= spanEnd) {
            SDL_RenderDrawLine(renderer_, spanStart, y, spanEnd, y);
        }
    }
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
