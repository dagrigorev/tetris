#include "platform/sdl2/Sdl2InputSource.hpp"

#include <SDL2/SDL.h>

namespace platform::sdl2 {

namespace {

void pushPressed(gamecore::InputFrame& frame, const gamecore::InputCommand command) {
    frame.pressedCommands.push_back(command);
}

void pushHeld(gamecore::InputFrame& frame, const gamecore::InputCommand command) {
    frame.heldCommands.push_back(command);
}

} // namespace

auto Sdl2InputSource::poll() -> gamecore::InputFrame {
    gamecore::InputFrame frame;

    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            pushPressed(frame, gamecore::InputCommand::Quit);
            continue;
        }

        if (event.type != SDL_KEYDOWN || event.key.repeat != 0) {
            continue;
        }

        // Edge-triggered commands: these actions must happen once per key press.
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: pushPressed(frame, gamecore::InputCommand::Back); break;
            case SDLK_q: pushPressed(frame, gamecore::InputCommand::Quit); break;
            case SDLK_DOWN:
            case SDLK_s:
                pushPressed(frame, gamecore::InputCommand::MenuDown);
                break;
            case SDLK_SPACE:
                pushPressed(frame, gamecore::InputCommand::HardDrop);
                pushPressed(frame, gamecore::InputCommand::Select);
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                pushPressed(frame, gamecore::InputCommand::Select);
                break;
            case SDLK_UP:
            case SDLK_w:
                pushPressed(frame, gamecore::InputCommand::RotateClockwise);
                pushPressed(frame, gamecore::InputCommand::MenuUp);
                break;
            case SDLK_x: pushPressed(frame, gamecore::InputCommand::RotateClockwise); break;
            case SDLK_z: pushPressed(frame, gamecore::InputCommand::RotateCounterClockwise); break;
            case SDLK_p: pushPressed(frame, gamecore::InputCommand::Pause); break;
            case SDLK_r: pushPressed(frame, gamecore::InputCommand::Restart); break;
            default: break;
        }
    }

    // Level-triggered movement: these commands are emitted every frame while the
    // physical key remains pressed. Game logic can therefore continue movement
    // without depending on OS key repeat or key release events.
    const auto* keyboard = SDL_GetKeyboardState(nullptr);
    if (keyboard[SDL_SCANCODE_LEFT] != 0 || keyboard[SDL_SCANCODE_A] != 0) {
        pushHeld(frame, gamecore::InputCommand::MoveLeft);
    }
    if (keyboard[SDL_SCANCODE_RIGHT] != 0 || keyboard[SDL_SCANCODE_D] != 0) {
        pushHeld(frame, gamecore::InputCommand::MoveRight);
    }
    if (keyboard[SDL_SCANCODE_UP] != 0 || keyboard[SDL_SCANCODE_W] != 0) {
        pushHeld(frame, gamecore::InputCommand::MoveUp);
    }
    if (keyboard[SDL_SCANCODE_DOWN] != 0 || keyboard[SDL_SCANCODE_S] != 0) {
        pushHeld(frame, gamecore::InputCommand::MoveDown);
        pushHeld(frame, gamecore::InputCommand::SoftDrop);
    }

    return frame;
}

} // namespace platform::sdl2
