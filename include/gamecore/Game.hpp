#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>

#include "gamecore/Input.hpp"
#include "gamecore/Renderer2D.hpp"

namespace gamecore {

using Seconds = std::chrono::duration<double>;

class IGame {
public:
    virtual ~IGame() = default;

    [[nodiscard]] virtual auto name() const -> std::string_view = 0;
    virtual void restart() = 0;
    virtual void handleInput(const InputFrame& input) = 0;
    virtual void update(Seconds deltaTime) = 0;
    virtual void render(IRenderer2D& renderer) const = 0;
    [[nodiscard]] virtual auto wantsToQuit() const -> bool = 0;
};

class IGameFactory {
public:
    virtual ~IGameFactory() = default;

    [[nodiscard]] virtual auto id() const -> std::string_view = 0;
    [[nodiscard]] virtual auto displayName() const -> std::string_view = 0;
    [[nodiscard]] virtual auto create() const -> std::unique_ptr<IGame> = 0;
};

} // namespace gamecore
