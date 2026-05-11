#include "gamecore/GameRegistry.hpp"

#include <algorithm>
#include <stdexcept>

namespace gamecore {

void GameRegistry::registerFactory(std::unique_ptr<IGameFactory> factory) {
    factories_.emplace(std::string{factory->id()}, std::move(factory));
}

auto GameRegistry::create(std::string_view id) const -> std::unique_ptr<IGame> {
    const auto found = factories_.find(std::string{id});
    if (found == factories_.end()) {
        throw std::runtime_error("Game factory not registered");
    }

    return found->second->create();
}

auto GameRegistry::availableGameIds() const -> std::vector<std::string> {
    std::vector<std::string> ids;
    ids.reserve(factories_.size());

    for (const auto& [id, _] : factories_) {
        ids.push_back(id);
    }

    return ids;
}

auto InputFrame::contains(const InputCommand command) const -> bool {
    return std::ranges::find(commands, command) != commands.end();
}

} // namespace gamecore
