#include "games/battle_city/BattleCityGameFactory.hpp"

#include "games/battle_city/BattleCityGame.hpp"

namespace games::battle_city {

BattleCityGameFactory::BattleCityGameFactory(gamecore::EventBus& events) : events_(events) {}

auto BattleCityGameFactory::id() const -> std::string_view { return "battle_city"; }
auto BattleCityGameFactory::displayName() const -> std::string_view { return "Battle City"; }

auto BattleCityGameFactory::create() const -> std::unique_ptr<gamecore::IGame> {
    return std::make_unique<BattleCityGame>(events_);
}

} // namespace games::battle_city
