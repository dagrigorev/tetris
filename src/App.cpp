#include "App.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

namespace tetris {
namespace {

[[nodiscard]] Game::Seconds readPerformanceSeconds(Uint64 current, Uint64 previous, double frequency) noexcept {
    return Game::Seconds{static_cast<double>(current - previous) / frequency};
}

} // namespace

App::App() {
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    window_ = SDL_CreateWindow(
        "Tetris C++23",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        Renderer::WindowWidth,
        Renderer::WindowHeight,
        SDL_WINDOW_SHOWN
    );

    if (window_ == nullptr) {
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    sdlRenderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer_ == nullptr) {
        sdlRenderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }

    if (sdlRenderer_ == nullptr) {
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }

    renderer_ = std::make_unique<Renderer>(sdlRenderer_);
    updateWindowTitle();
}

App::~App() {
    renderer_.reset();

    if (sdlRenderer_ != nullptr) {
        SDL_DestroyRenderer(sdlRenderer_);
        sdlRenderer_ = nullptr;
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

int App::run() {
    bool quit = false;
    auto previousCounter = SDL_GetPerformanceCounter();
    const auto frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (!quit) {
        const auto currentCounter = SDL_GetPerformanceCounter();
        const auto delta = readPerformanceSeconds(currentCounter, previousCounter, frequency);
        previousCounter = currentCounter;

        handleEvents(quit);
        game_.update(delta);
        updateWindowTitle();
        renderer_->render(game_);
    }

    return 0;
}

void App::handleEvents(bool& quit) {
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
            continue;
        }

        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            handleKeyDown(event.key.keysym.sym, quit);
        }
    }
}

void App::handleKeyDown(SDL_Keycode key, bool& quit) {
    switch (key) {
        case SDLK_ESCAPE:
            quit = true;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            game_.moveLeft();
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            game_.moveRight();
            break;
        case SDLK_DOWN:
        case SDLK_s:
            game_.softDrop();
            break;
        case SDLK_UP:
        case SDLK_w:
        case SDLK_x:
            game_.rotateClockwise();
            break;
        case SDLK_z:
            game_.rotateCounterClockwise();
            break;
        case SDLK_SPACE:
            game_.hardDrop();
            break;
        case SDLK_p:
            game_.togglePause();
            break;
        case SDLK_r:
            game_.reset();
            break;
        default:
            break;
    }
}

void App::updateWindowTitle() {
    const auto title = game_.titleText();
    if (title != lastTitle_) {
        SDL_SetWindowTitle(window_, title.c_str());
        lastTitle_ = title;
    }
}

} // namespace tetris
