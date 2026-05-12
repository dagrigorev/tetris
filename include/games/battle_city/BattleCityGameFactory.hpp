#pragma once

#include <memory>
#include <string_view>

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"

namespace games::battle_city {

class BattleCityGameFactory final : public gamecore::IGameFactory {
public:
    explicit BattleCityGameFactory(gamecore::EventBus& events);

    [[nodiscard]] auto id() const -> std::string_view override;
    [[nodiscard]] auto displayName() const -> std::string_view override;
    [[nodiscard]] auto create() const -> std::unique_ptr<gamecore::IGame> override;

private:
    gamecore::EventBus& events_;
};

} // namespace games::battle_city
