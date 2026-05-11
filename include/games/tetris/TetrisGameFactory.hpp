#pragma once

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"

namespace games::tetris {

class TetrisGameFactory final : public gamecore::IGameFactory {
public:
    explicit TetrisGameFactory(gamecore::EventBus& events);

    [[nodiscard]] auto id() const -> std::string_view override;
    [[nodiscard]] auto displayName() const -> std::string_view override;
    [[nodiscard]] auto create() const -> std::unique_ptr<gamecore::IGame> override;

private:
    gamecore::EventBus& events_;
};

} // namespace games::tetris
