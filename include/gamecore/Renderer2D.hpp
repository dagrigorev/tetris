#pragma once

#include <string_view>

#include "gamecore/Color.hpp"
#include "gamecore/Geometry.hpp"

namespace gamecore {

class IRenderer2D {
public:
    virtual ~IRenderer2D() = default;

    virtual void beginFrame(Color clearColor) = 0;
    virtual void endFrame() = 0;

    virtual void fillRect(Recti rect, Color color) = 0;
    virtual void drawRect(Recti rect, Color color) = 0;
    virtual void drawLine(Vec2i from, Vec2i to, Color color) = 0;
    virtual void fillCircle(Vec2i center, int radius, Color color) = 0;
    virtual void drawCircle(Vec2i center, int radius, Color color) = 0;
    virtual void fillTriangle(Vec2i a, Vec2i b, Vec2i c, Color color) = 0;
    virtual void drawText(Vec2i position, std::string_view text, Color color, int scale = 2) = 0;
};

} // namespace gamecore
