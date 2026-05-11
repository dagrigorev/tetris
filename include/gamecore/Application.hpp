#pragma once

#include <memory>

#include "gamecore/Game.hpp"
#include "gamecore/Input.hpp"
#include "gamecore/Renderer2D.hpp"

namespace gamecore {

class Application final {
public:
    Application(std::unique_ptr<IGame> game, IInputSource& input, IRenderer2D& renderer);

    auto run() -> int;

private:
    std::unique_ptr<IGame> game_;
    IInputSource& input_;
    IRenderer2D& renderer_;
};

} // namespace gamecore
