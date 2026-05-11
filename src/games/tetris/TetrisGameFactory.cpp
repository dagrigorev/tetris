#include "games/tetris/TetrisGameFactory.hpp"

#include <memory>

#include "games/tetris/ScoringStrategy.hpp"
#include "games/tetris/SevenBagTetrominoProvider.hpp"
#include "games/tetris/TetrisGame.hpp"

namespace games::tetris {

TetrisGameFactory::TetrisGameFactory(gamecore::EventBus& events) : events_(events) {}

auto TetrisGameFactory::id() const -> std::string_view {
    return "tetris";
}

auto TetrisGameFactory::displayName() const -> std::string_view {
    return "Tetris";
}

auto TetrisGameFactory::create() const -> std::unique_ptr<gamecore::IGame> {
    return std::make_unique<TetrisGame>(
        std::make_unique<SevenBagTetrominoProvider>(),
        std::make_unique<ClassicScoringStrategy>(),
        events_
    );
}

} // namespace games::tetris
