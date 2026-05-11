#include "gamecore/EventBus.hpp"

#include <string>

namespace gamecore {

void EventBus::subscribe(std::string_view eventName, Handler handler) {
    handlers_[std::string{eventName}].push_back(std::move(handler));
}

void EventBus::publish(GameEvent event) const {
    const auto found = handlers_.find(std::string{event.name});
    if (found == handlers_.end()) {
        return;
    }

    for (const auto& handler : found->second) {
        handler(event);
    }
}

} // namespace gamecore
