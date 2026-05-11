#include "platform/sdl2/Sdl2Window.hpp"

#include <stdexcept>
#include <string>

namespace platform::sdl2 {

namespace {

[[nodiscard]] auto sdlError(std::string_view prefix) -> std::runtime_error {
    return std::runtime_error(std::string{prefix} + ": " + SDL_GetError());
}

} // namespace

Sdl2Window::Sdl2Window(std::string_view title, const int width, const int height) {
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        throw sdlError("SDL_Init failed");
    }

    window_ = SDL_CreateWindow(
        std::string{title}.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window_ == nullptr) {
        SDL_Quit();
        throw sdlError("SDL_CreateWindow failed");
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw sdlError("SDL_CreateRenderer failed");
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
}

Sdl2Window::~Sdl2Window() {
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

} // namespace platform::sdl2
