#include <exception>
#include <iostream>
#include <memory>

#include "gamecore/Application.hpp"
#include "gamecore/EventBus.hpp"
#include "gamecore/GameRegistry.hpp"
#include "games/tetris/TetrisGameFactory.hpp"
#include "platform/sdl2/Sdl2InputSource.hpp"
#include "platform/sdl2/Sdl2Renderer2D.hpp"
#include "platform/sdl2/Sdl2Window.hpp"

int main(int argc, char** argv) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    try {
        gamecore::EventBus events;
        events.subscribe("tetris.game_over", [](const gamecore::GameEvent& event) {
            std::cout << "Game over. Score: " << event.value << '\n';
        });

        gamecore::GameRegistry registry;
        registry.registerFactory(std::make_unique<games::tetris::TetrisGameFactory>(events));

        auto game = registry.create("tetris");

        platform::sdl2::Sdl2Window window{"Layered Games - Tetris", 760, 640};
        platform::sdl2::Sdl2InputSource input;
        platform::sdl2::Sdl2Renderer2D renderer{window.nativeRenderer()};

        gamecore::Application app{std::move(game), input, renderer};
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
