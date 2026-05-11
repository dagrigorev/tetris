#pragma once

#include "gamecore/Input.hpp"

namespace platform::sdl2 {

class Sdl2InputSource final : public gamecore::IInputSource {
public:
    [[nodiscard]] auto poll() -> gamecore::InputFrame override;
};

} // namespace platform::sdl2
