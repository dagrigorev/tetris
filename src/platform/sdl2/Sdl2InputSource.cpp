#include "platform/sdl2/Sdl2InputSource.hpp"

#include <SDL2/SDL.h>

namespace platform::sdl2 {

namespace {

void push(gamecore::InputFrame& frame, const gamecore::InputCommand command) {
    frame.commands.push_back(command);
}

} // namespace

auto Sdl2InputSource::poll() -> gamecore::InputFrame {
    gamecore::InputFrame frame;

    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            push(frame, gamecore::InputCommand::Quit);
            continue;
        }

        if (event.type != SDL_KEYDOWN || event.key.repeat != 0) {
            continue;
        }

        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: push(frame, gamecore::InputCommand::Quit); break;
            case SDLK_LEFT:
            case SDLK_a: push(frame, gamecore::InputCommand::MoveLeft); break;
            case SDLK_RIGHT:
            case SDLK_d: push(frame, gamecore::InputCommand::MoveRight); break;
            case SDLK_DOWN:
            case SDLK_s: push(frame, gamecore::InputCommand::SoftDrop); break;
            case SDLK_SPACE: push(frame, gamecore::InputCommand::HardDrop); break;
            case SDLK_UP:
            case SDLK_w:
            case SDLK_x: push(frame, gamecore::InputCommand::RotateClockwise); break;
            case SDLK_z: push(frame, gamecore::InputCommand::RotateCounterClockwise); break;
            case SDLK_p: push(frame, gamecore::InputCommand::Pause); break;
            case SDLK_r: push(frame, gamecore::InputCommand::Restart); break;
            default: break;
        }
    }

    return frame;
}

} // namespace platform::sdl2
