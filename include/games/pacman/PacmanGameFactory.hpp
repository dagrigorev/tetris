#pragma once

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"

namespace games::pacman {

class PacmanGameFactory final : public gamecore::IGameFactory {
public:
    explicit PacmanGameFactory(gamecore::EventBus& events);

    [[nodiscard]] auto id() const -> std::string_view override;
    [[nodiscard]] auto displayName() const -> std::string_view override;
    [[nodiscard]] auto create() const -> std::unique_ptr<gamecore::IGame> override;

private:
    gamecore::EventBus& events_;
};

} // namespace games::pacman
