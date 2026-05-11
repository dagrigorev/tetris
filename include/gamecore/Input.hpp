#pragma once

#include <vector>

namespace gamecore {

enum class InputCommand {
    MoveLeft,
    MoveRight,
    SoftDrop,
    HardDrop,
    RotateClockwise,
    RotateCounterClockwise,
    Pause,
    Restart,
    Quit
};

struct InputFrame final {
    std::vector<InputCommand> commands;

    [[nodiscard]] auto contains(const InputCommand command) const -> bool;
};

class IInputSource {
public:
    virtual ~IInputSource() = default;
    [[nodiscard]] virtual auto poll() -> InputFrame = 0;
};

} // namespace gamecore
