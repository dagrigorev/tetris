#include "games/tetris/TetrisGame.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

namespace games::tetris {
namespace {

[[nodiscard]] auto colorFromCell(const int value) -> gamecore::Color {
    if (value <= 0) {
        return gamecore::Black;
    }
    return colorOf(static_cast<TetrominoKind>(value - 1));
}

[[nodiscard]] auto pieceValue(const TetrominoKind kind) -> int {
    return static_cast<int>(std::to_underlying(kind)) + 1;
}

[[nodiscard]] auto translucent(gamecore::Color c, std::uint8_t alpha) -> gamecore::Color {
    c.a = alpha;
    return c;
}

} // namespace

TetrisGame::TetrisGame(std::unique_ptr<ITetrominoProvider> provider,
                       std::unique_ptr<IScoringStrategy> scoring,
                       gamecore::EventBus& events)
    : provider_(std::move(provider)), scoring_(std::move(scoring)), events_(events) {
    restart();
}

auto TetrisGame::name() const -> std::string_view {
    return "Tetris";
}

void TetrisGame::restart() {
    board_.clear();
    score_ = {};
    phase_ = TetrisPhase::Playing;
    quit_ = false;
    gravityAccumulator_ = gamecore::Seconds::zero();
    horizontalDirection_ = 0;
    horizontalHoldTime_ = gamecore::Seconds::zero();
    horizontalRepeatAccumulator_ = gamecore::Seconds::zero();
    softDropHeld_ = false;
    softDropRepeatAccumulator_ = gamecore::Seconds::zero();

    while (!nextQueue_.empty()) {
        nextQueue_.pop();
    }

    fillPreviewQueue();
    spawnNext();
}

void TetrisGame::handleInput(const gamecore::InputFrame& input) {
    using enum gamecore::InputCommand;

    if (input.isPressed(Quit)) {
        quit_ = true;
        return;
    }

    if (input.isPressed(Restart)) {
        restart();
        return;
    }

    if (input.isPressed(Pause) && phase_ != TetrisPhase::GameOver) {
        phase_ = phase_ == TetrisPhase::Paused ? TetrisPhase::Playing : TetrisPhase::Paused;
    }

    if (phase_ != TetrisPhase::Playing) {
        horizontalDirection_ = 0;
        softDropHeld_ = false;
        return;
    }

    int requestedHorizontalDirection = 0;
    if (input.isDown(MoveLeft)) {
        requestedHorizontalDirection -= 1;
    }
    if (input.isDown(MoveRight)) {
        requestedHorizontalDirection += 1;
    }

    if (requestedHorizontalDirection != horizontalDirection_) {
        horizontalDirection_ = requestedHorizontalDirection;
        horizontalHoldTime_ = gamecore::Seconds::zero();
        horizontalRepeatAccumulator_ = gamecore::Seconds::zero();
        if (horizontalDirection_ != 0) {
            (void)tryMove({horizontalDirection_, 0});
        }
    }

    if (input.isPressed(RotateClockwise)) {
        (void)tryRotate(1);
    }
    if (input.isPressed(RotateCounterClockwise)) {
        (void)tryRotate(-1);
    }

    const bool requestedSoftDrop = input.isDown(SoftDrop);
    if (requestedSoftDrop != softDropHeld_) {
        softDropHeld_ = requestedSoftDrop;
        softDropRepeatAccumulator_ = gamecore::Seconds::zero();
        if (softDropHeld_) {
            softDropStep();
        }
    }

    if (input.isPressed(HardDrop)) {
        hardDrop();
    }
}

void TetrisGame::update(const gamecore::Seconds deltaTime) {
    if (phase_ != TetrisPhase::Playing) {
        return;
    }

    handleHorizontalHold(deltaTime);
    handleSoftDropHold(deltaTime);

    gravityAccumulator_ += deltaTime;
    const auto interval = gravityInterval();

    while (gravityAccumulator_ >= interval) {
        gravityAccumulator_ -= interval;
        if (!tryMove({0, 1})) {
            lockActive();
            break;
        }
    }
}

void TetrisGame::render(gamecore::IRenderer2D& renderer) const {
    renderer.beginFrame({14, 14, 18, 255});

    const auto boardBackground = gamecore::Recti{
        BoardOriginX - 6,
        BoardOriginY - 6,
        BoardWidth * CellSize + 12,
        BoardHeight * CellSize + 12
    };
    renderer.fillRect(boardBackground, {28, 28, 36, 255});
    renderer.drawRect(boardBackground, {100, 100, 120, 255});

    for (int y = 0; y < BoardHeight; ++y) {
        for (int x = 0; x < BoardWidth; ++x) {
            const auto rect = boardRect({x, y});
            renderer.fillRect(rect, {20, 20, 25, 255});
            renderer.drawRect(rect, {38, 38, 45, 255});

            const auto value = board_.cell({x, y});
            if (value != 0) {
                renderer.fillRect(rect, colorFromCell(value));
                renderer.drawRect(rect, {235, 235, 235, 80});
            }
        }
    }

    const auto ghost = ghostPiece();
    for (const auto block : ghost.blocks()) {
        if (block.y >= 0) {
            renderer.drawRect(boardRect(block), translucent(active_.color(), 100));
        }
    }

    for (const auto block : active_.blocks()) {
        if (block.y >= 0) {
            renderer.fillRect(boardRect(block), active_.color());
            renderer.drawRect(boardRect(block), {245, 245, 245, 150});
        }
    }

    const int sideX = SidebarX;
    renderer.drawText({sideX, BoardOriginY}, "LAYERED TETRIS", {240, 240, 245, 255}, 2);
    renderer.drawText({sideX, BoardOriginY + 42}, std::string{"SCORE "} + std::to_string(score_.score), {210, 210, 225, 255}, 2);
    renderer.drawText({sideX, BoardOriginY + 70}, std::string{"LINES "} + std::to_string(score_.lines), {210, 210, 225, 255}, 2);
    renderer.drawText({sideX, BoardOriginY + 98}, std::string{"LEVEL "} + std::to_string(score_.level), {210, 210, 225, 255}, 2);

    const auto previewPanel = gamecore::Recti{sideX - 10, BoardOriginY + 132, 172, 352};
    renderer.fillRect(previewPanel, {22, 22, 30, 255});
    renderer.drawRect(previewPanel, {70, 70, 86, 255});
    renderer.drawText({sideX, BoardOriginY + 148}, "NEXT", {240, 240, 245, 255}, 2);

    auto queueCopy = nextQueue_;
    for (int index = 0; index < PreviewCount && !queueCopy.empty(); ++index) {
        const auto kind = queueCopy.front();
        queueCopy.pop();

        const Tetromino preview{kind, {0, 0}, 0};
        for (const auto block : preview.blocks()) {
            renderer.fillRect(previewRect(block, index), colorOf(kind));
            renderer.drawRect(previewRect(block, index), {245, 245, 245, 120});
        }
    }

    const auto controlsPanel = gamecore::Recti{sideX - 10, ControlsTopY - 14, 246, 82};
    renderer.fillRect(controlsPanel, {22, 22, 30, 255});
    renderer.drawRect(controlsPanel, {70, 70, 86, 255});
    renderer.drawText({sideX, ControlsTopY}, "P PAUSE  R RESTART", {150, 150, 165, 255}, 1);
    renderer.drawText({sideX, ControlsTopY + ControlsLineHeight}, "ARROWS/WASD MOVE", {150, 150, 165, 255}, 1);
    renderer.drawText({sideX, ControlsTopY + ControlsLineHeight * 2}, "SPACE HARD DROP", {150, 150, 165, 255}, 1);

    if (phase_ == TetrisPhase::Paused) {
        renderer.fillRect({BoardOriginX + 18, BoardOriginY + 230, 244, 62}, {0, 0, 0, 210});
        renderer.drawRect({BoardOriginX + 18, BoardOriginY + 230, 244, 62}, gamecore::White);
        renderer.drawText({BoardOriginX + 70, BoardOriginY + 252}, "PAUSED", gamecore::White, 3);
    }

    if (phase_ == TetrisPhase::GameOver) {
        renderer.fillRect({BoardOriginX + 4, BoardOriginY + 210, 272, 116}, {0, 0, 0, 225});
        renderer.drawRect({BoardOriginX + 4, BoardOriginY + 210, 272, 116}, {240, 60, 60, 255});
        renderer.drawText({BoardOriginX + 35, BoardOriginY + 238}, "GAME OVER", {255, 90, 90, 255}, 3);
        renderer.drawText({BoardOriginX + 54, BoardOriginY + 288}, "R TO RESTART", gamecore::White, 2);
    }

    renderer.endFrame();
}

auto TetrisGame::wantsToQuit() const -> bool {
    return quit_;
}

void TetrisGame::fillPreviewQueue() {
    while (static_cast<int>(nextQueue_.size()) < PreviewCount) {
        nextQueue_.push(provider_->next());
    }
}

void TetrisGame::spawnNext() {
    fillPreviewQueue();

    const auto kind = nextQueue_.front();
    nextQueue_.pop();
    active_ = Tetromino{kind, spawnPosition(kind), 0};
    fillPreviewQueue();

    if (!isValid(active_)) {
        phase_ = TetrisPhase::GameOver;
        events_.publish({"tetris.game_over", score_.score});
    }
}

auto TetrisGame::isValid(const Tetromino& piece) const -> bool {
    for (const auto block : piece.blocks()) {
        if (block.x < 0 || block.x >= BoardWidth || block.y >= BoardHeight) {
            return false;
        }
        if (block.y >= 0 && board_.isOccupied(block)) {
            return false;
        }
    }
    return true;
}

auto TetrisGame::tryMove(const gamecore::Vec2i offset) -> bool {
    auto moved = active_;
    moved.position = moved.position + offset;

    if (!isValid(moved)) {
        return false;
    }

    active_ = moved;
    return true;
}

auto TetrisGame::tryRotate(const int direction) -> bool {
    const auto rotated = active_.rotated(direction);
    static constexpr std::array<gamecore::Vec2i, 7> kicks{
        gamecore::Vec2i{0, 0},
        gamecore::Vec2i{-1, 0},
        gamecore::Vec2i{1, 0},
        gamecore::Vec2i{-2, 0},
        gamecore::Vec2i{2, 0},
        gamecore::Vec2i{0, -1},
        gamecore::Vec2i{0, 1}
    };

    for (const auto kick : kicks) {
        auto candidate = rotated;
        candidate.position = candidate.position + kick;
        if (isValid(candidate)) {
            active_ = candidate;
            return true;
        }
    }

    return false;
}

void TetrisGame::hardDrop() {
    int cells = 0;
    while (tryMove({0, 1})) {
        ++cells;
    }
    scoring_->applyHardDrop(score_, cells);
    lockActive();
}

void TetrisGame::lockActive() {
    for (const auto block : active_.blocks()) {
        if (block.y >= 0) {
            board_.setCell(block, pieceValue(active_.kind));
        }
    }

    const auto cleared = board_.clearFullLines();
    scoring_->applyLineClear(score_, cleared);
    if (cleared > 0) {
        events_.publish({"tetris.lines_cleared", cleared});
    }

    spawnNext();
}

auto TetrisGame::ghostPiece() const -> Tetromino {
    auto ghost = active_;
    while (true) {
        auto moved = ghost;
        moved.position.y += 1;
        if (!isValid(moved)) {
            return ghost;
        }
        ghost = moved;
    }
}

auto TetrisGame::gravityInterval() const -> gamecore::Seconds {
    const auto milliseconds = std::max(80, 650 - ((score_.level - 1) * 45));
    return std::chrono::duration_cast<gamecore::Seconds>(std::chrono::milliseconds(milliseconds));
}

void TetrisGame::handleHorizontalHold(const gamecore::Seconds deltaTime) {
    if (horizontalDirection_ == 0) {
        return;
    }

    static constexpr auto initialDelay = gamecore::Seconds{0.16};
    static constexpr auto repeatInterval = gamecore::Seconds{0.055};

    horizontalHoldTime_ += deltaTime;
    if (horizontalHoldTime_ < initialDelay) {
        return;
    }

    horizontalRepeatAccumulator_ += deltaTime;
    while (horizontalRepeatAccumulator_ >= repeatInterval) {
        horizontalRepeatAccumulator_ -= repeatInterval;
        (void)tryMove({horizontalDirection_, 0});
    }
}

void TetrisGame::handleSoftDropHold(const gamecore::Seconds deltaTime) {
    if (!softDropHeld_) {
        return;
    }

    static constexpr auto repeatInterval = gamecore::Seconds{0.035};

    softDropRepeatAccumulator_ += deltaTime;
    while (softDropRepeatAccumulator_ >= repeatInterval) {
        softDropRepeatAccumulator_ -= repeatInterval;
        softDropStep();
    }
}

void TetrisGame::softDropStep() {
    if (tryMove({0, 1})) {
        scoring_->applySoftDrop(score_);
    }
}

auto TetrisGame::boardRect(const gamecore::Vec2i cell) -> gamecore::Recti {
    return {
        BoardOriginX + cell.x * CellSize,
        BoardOriginY + cell.y * CellSize,
        CellSize - 1,
        CellSize - 1
    };
}

auto TetrisGame::previewRect(const gamecore::Vec2i cell, const int index) -> gamecore::Recti {
    const int x = SidebarX + 16 + cell.x * PreviewCellSize;
    const int y = PreviewTopY + index * PreviewSlotHeight + cell.y * PreviewCellSize;
    return {x, y, PreviewCellSize - 1, PreviewCellSize - 1};
}

} // namespace games::tetris
