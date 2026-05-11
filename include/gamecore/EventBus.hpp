#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace gamecore {

struct GameEvent final {
    std::string_view name;
    int value{};
};

class EventBus final {
public:
    using Handler = std::function<void(const GameEvent&)>;

    void subscribe(std::string_view eventName, Handler handler);
    void publish(GameEvent event) const;

private:
    std::unordered_map<std::string, std::vector<Handler>> handlers_;
};

} // namespace gamecore
