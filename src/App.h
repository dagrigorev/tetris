#pragma once

#include "Game.h"
#include "Renderer.h"

#include <SDL.h>

#include <memory>
#include <string>

namespace tetris {

class App final {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    [[nodiscard]] int run();

private:
    void handleEvents(bool& quit);
    void handleKeyDown(SDL_Keycode key, bool& quit);
    void updateWindowTitle();

    SDL_Window* window_{};
    SDL_Renderer* sdlRenderer_{};
    std::unique_ptr<Renderer> renderer_;
    Game game_;
    std::string lastTitle_;
};

} // namespace tetris
