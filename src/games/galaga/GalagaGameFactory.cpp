#include "games/galaga/GalagaGameFactory.hpp"

#include "games/galaga/GalagaGame.hpp"

namespace games::galaga {

GalagaGameFactory::GalagaGameFactory(gamecore::EventBus& events) : events_(events) {}

auto GalagaGameFactory::id() const -> std::string_view {
    return "galaga";
}

auto GalagaGameFactory::displayName() const -> std::string_view {
    return "GALAGA";
}

auto GalagaGameFactory::create() const -> std::unique_ptr<gamecore::IGame> {
    return std::make_unique<GalagaGame>(events_);
}

} // namespace games::galaga
