#include "games/arkanoid/ArkanoidGameFactory.hpp"

#include "games/arkanoid/ArkanoidGame.hpp"

namespace games::arkanoid {

ArkanoidGameFactory::ArkanoidGameFactory(gamecore::EventBus& events) : events_(events) {}

auto ArkanoidGameFactory::id() const -> std::string_view {
    return "arkanoid";
}

auto ArkanoidGameFactory::displayName() const -> std::string_view {
    return "ARKANOID";
}

auto ArkanoidGameFactory::create() const -> std::unique_ptr<gamecore::IGame> {
    return std::make_unique<ArkanoidGame>(events_);
}

} // namespace games::arkanoid
