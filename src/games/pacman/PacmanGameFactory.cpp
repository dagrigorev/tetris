#include "games/pacman/PacmanGameFactory.hpp"

#include "games/pacman/PacmanGame.hpp"

namespace games::pacman {

PacmanGameFactory::PacmanGameFactory(gamecore::EventBus& events) : events_(events) {}

auto PacmanGameFactory::id() const -> std::string_view {
    return "pacman";
}

auto PacmanGameFactory::displayName() const -> std::string_view {
    return "PACMAN";
}

auto PacmanGameFactory::create() const -> std::unique_ptr<gamecore::IGame> {
    return std::make_unique<PacmanGame>(events_);
}

} // namespace games::pacman
