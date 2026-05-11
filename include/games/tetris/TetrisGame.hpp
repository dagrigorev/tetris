#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <queue>

#include "gamecore/Game.hpp"
#include "gamecore/EventBus.hpp"
#include "games/tetris/Board.hpp"
#include "games/tetris/ITetrominoProvider.hpp"
#include "games/tetris/ScoringStrategy.hpp"
#include "games/tetris/Tetromino.hpp"

namespace games::tetris {

enum class TetrisPhase {
    Playing,
    Paused,
    GameOver
};

class TetrisGame final : public gamecore::IGame {
public:
    TetrisGame(std::unique_ptr<ITetrominoProvider> provider,
               std::unique_ptr<IScoringStrategy> scoring,
               gamecore::EventBus& events);

    [[nodiscard]] auto name() const -> std::string_view override;
    void restart() override;
    void handleInput(const gamecore::InputFrame& input) override;
    void update(gamecore::Seconds deltaTime) override;
    void render(gamecore::IRenderer2D& renderer) const override;
    [[nodiscard]] auto wantsToQuit() const -> bool override;

private:
    static constexpr int PreviewCount = 5;
    static constexpr int CellSize = 28;
    static constexpr int BoardOriginX = 48;
    static constexpr int BoardOriginY = 36;

    static constexpr int SidebarGap = 42;
    static constexpr int SidebarX = BoardOriginX + BoardWidth * CellSize + SidebarGap;
    static constexpr int PreviewCellSize = 14;
    static constexpr int PreviewTopY = BoardOriginY + 176;
    static constexpr int PreviewSlotHeight = 56;
    static constexpr int ControlsTopY = BoardOriginY + 500;
    static constexpr int ControlsLineHeight = 18;

    Board board_;
    std::unique_ptr<ITetrominoProvider> provider_;
    std::unique_ptr<IScoringStrategy> scoring_;
    gamecore::EventBus& events_;

    Tetromino active_{};
    std::queue<TetrominoKind> nextQueue_;
    ScoreState score_{};
    TetrisPhase phase_{TetrisPhase::Playing};
    bool quit_{};
    gamecore::Seconds gravityAccumulator_{};

    void fillPreviewQueue();
    void spawnNext();
    [[nodiscard]] auto isValid(const Tetromino& piece) const -> bool;
    [[nodiscard]] auto tryMove(gamecore::Vec2i offset) -> bool;
    [[nodiscard]] auto tryRotate(int direction) -> bool;
    void hardDrop();
    void lockActive();
    [[nodiscard]] auto ghostPiece() const -> Tetromino;
    [[nodiscard]] auto gravityInterval() const -> gamecore::Seconds;

    static auto boardRect(gamecore::Vec2i cell) -> gamecore::Recti;
    static auto previewRect(gamecore::Vec2i cell, int index) -> gamecore::Recti;
};

} // namespace games::tetris
