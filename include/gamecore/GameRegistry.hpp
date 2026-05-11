#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gamecore/Game.hpp"

namespace gamecore {

class GameRegistry final {
public:
    void registerFactory(std::unique_ptr<IGameFactory> factory);

    [[nodiscard]] auto create(std::string_view id) const -> std::unique_ptr<IGame>;
    [[nodiscard]] auto availableGameIds() const -> std::vector<std::string>;

private:
    std::unordered_map<std::string, std::unique_ptr<IGameFactory>> factories_;
};

} // namespace gamecore
