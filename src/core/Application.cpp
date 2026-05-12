#include "gamecore/Application.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

namespace gamecore {

Application::Application(std::unique_ptr<IGame> game, IInputSource& input, IRenderer2D& renderer)
    : game_(std::move(game)), input_(input), renderer_(renderer) {}

auto Application::run() -> int {
    using clock = std::chrono::steady_clock;
    constexpr auto targetFrameTime = std::chrono::duration<double>{1.0 / 60.0};

    auto previous = clock::now();

    while (!game_->wantsToQuit()) {
        const auto now = clock::now();
        const auto delta = std::chrono::duration_cast<Seconds>(now - previous);
        previous = now;

        auto input = input_.poll();
        if (input.isPressed(InputCommand::Quit)) {
            break;
        }

        game_->handleInput(input);
        game_->update(delta);
        game_->render(renderer_);

        const auto frameElapsed = clock::now() - now;
        if (frameElapsed < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - frameElapsed);
        }
    }

    return 0;
}

} // namespace gamecore
