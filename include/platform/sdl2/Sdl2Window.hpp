#pragma once

#include <string_view>

#include <SDL2/SDL.h>

namespace platform::sdl2 {

class Sdl2Window final {
public:
    Sdl2Window(std::string_view title, int width, int height);
    ~Sdl2Window();

    Sdl2Window(const Sdl2Window&) = delete;
    auto operator=(const Sdl2Window&) -> Sdl2Window& = delete;

    Sdl2Window(Sdl2Window&&) = delete;
    auto operator=(Sdl2Window&&) -> Sdl2Window& = delete;

    [[nodiscard]] auto nativeWindow() const noexcept -> SDL_Window* { return window_; }
    [[nodiscard]] auto nativeRenderer() const noexcept -> SDL_Renderer* { return renderer_; }

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
};

} // namespace platform::sdl2
