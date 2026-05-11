#pragma once

#include "Tetromino.h"

#include <array>
#include <deque>
#include <random>
#include <string>
#include <vector>

namespace tetris {

class Game {
public:
    static constexpr int BoardWidth = 10;
    static constexpr int BoardHeight = 20;
    static constexpr int PreviewCount = 5;

    enum class State {
        Running,
        Paused,
        GameOver
    };

    struct Piece {
        TetrominoType type{TetrominoType::I};
        int rotation{0};
        int x{3};
        int y{0};
    };

    Game();

    void reset();
    void update(double deltaSeconds);

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
    [[nodiscard]] double dropIntervalSeconds() const noexcept;

    State state_{State::Running};
    std::vector<int> board_;
    Piece active_{};
    std::deque<TetrominoType> queue_;
    std::vector<TetrominoType> bag_;
    std::mt19937 rng_;
    double dropAccumulator_{0.0};
    int score_{0};
    int lines_{0};
    int level_{1};
};

} // namespace tetris
