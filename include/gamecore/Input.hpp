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
    Quit,
    Back,
    Select,
    MenuUp,
    MenuDown
};

struct InputFrame final {
    // Edge-triggered commands: present only in the frame where the key was pressed.
    std::vector<InputCommand> pressedCommands;

    // Level-triggered commands: present every frame while the key is physically held down.
    std::vector<InputCommand> heldCommands;

    [[nodiscard]] auto isPressed(InputCommand command) const -> bool;
    [[nodiscard]] auto isDown(InputCommand command) const -> bool;

    // Backward-compatible helper: true for either a press or a held key.
    [[nodiscard]] auto contains(InputCommand command) const -> bool;
};

class IInputSource {
public:
    virtual ~IInputSource() = default;
    [[nodiscard]] virtual auto poll() -> InputFrame = 0;
};

} // namespace gamecore
