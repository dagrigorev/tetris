#pragma once

#include "Tetromino.h"

#include <array>
#include <chrono>
#include <deque>
#include <random>
#include <string>
#include <vector>

namespace tetris {

class Game final {
public:
    using Seconds = std::chrono::duration<double>;

    static constexpr int BoardWidth = 10;
    static constexpr int BoardHeight = 20;
    static constexpr int PreviewCount = 5;

    enum class State {
        Running,
        Paused,
        GameOver
    };

    struct Piece final {
        TetrominoType type{TetrominoType::I};
        int rotation{0};
        int x{3};
        int y{0};
    };

    Game();

    void reset();
    void update(Seconds delta);

    void moveLeft();
    void moveRight();
    void softDrop();
    void hardDrop();
    void rotateClockwise();
    void rotateCounterClockwise();
    void togglePause();

    [[nodiscard]] State state() const noexcept { return state_; }
    [[nodiscard]] const std::vector<int>& board() const noexcept { return board_; }
    [[nodiscard]] const Piece& activePiece() const noexcept { return active_; }
    [[nodiscard]] Piece ghostPiece() const;
    [[nodiscard]] std::array<TetrominoType, PreviewCount> nextPreview() const;

    [[nodiscard]] int score() const noexcept { return score_; }
    [[nodiscard]] int lines() const noexcept { return lines_; }
    [[nodiscard]] int level() const noexcept { return level_; }
    [[nodiscard]] std::string titleText() const;

private:
    [[nodiscard]] static constexpr std::size_t boardIndex(int x, int y) noexcept {
        return static_cast<std::size_t>(y * BoardWidth + x);
    }

    [[nodiscard]] bool isCellInside(int x, int y) const noexcept;
    [[nodiscard]] bool isCellOccupied(int x, int y) const noexcept;
    [[nodiscard]] bool canPlace(const Piece& piece) const noexcept;

    void moveDownByTimer();
    void lockActivePiece();
    int clearCompletedLines();
    void spawnNextPiece();
    void fillQueue();
    TetrominoType takeFromBag();
    void addScoreForClearedLines(int count);
    [[nodiscard]] Seconds dropInterval() const noexcept;

    State state_{State::Running};
    std::vector<int> board_;
    Piece active_{};
    std::deque<TetrominoType> queue_;
    std::vector<TetrominoType> bag_;
    std::mt19937 rng_;
    Seconds dropAccumulator_{Seconds::zero()};
    int score_{0};
    int lines_{0};
    int level_{1};
};

} // namespace tetris
