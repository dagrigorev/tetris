#include "Game.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>

namespace tetris {
namespace {

constexpr std::array<int, 5> kLineScores{0, 100, 300, 500, 800};
constexpr std::array<int, 5> kWallKickOffsets{0, -1, 1, -2, 2};
constexpr auto kMinDropInterval = Game::Seconds{0.08};
constexpr auto kInitialDropInterval = Game::Seconds{0.70};
constexpr auto kLevelDropStep = Game::Seconds{0.055};

} // namespace

Game::Game()
    : board_(BoardWidth * BoardHeight, 0),
      rng_(std::random_device{}()) {
    reset();
}

void Game::reset() {
    std::ranges::fill(board_, 0);
    queue_.clear();
    bag_.clear();
    dropAccumulator_ = Seconds::zero();
    score_ = 0;
    lines_ = 0;
    level_ = 1;
    state_ = State::Running;
    fillQueue();
    spawnNextPiece();
}

void Game::update(Seconds delta) {
    if (state_ != State::Running) {
        return;
    }

    dropAccumulator_ += delta;
    while (dropAccumulator_ >= dropInterval() && state_ == State::Running) {
        moveDownByTimer();
        dropAccumulator_ -= dropInterval();
    }
}

void Game::moveLeft() {
    if (state_ != State::Running) {
        return;
    }

    auto candidate = active_;
    --candidate.x;
    if (canPlace(candidate)) {
        active_ = candidate;
    }
}

void Game::moveRight() {
    if (state_ != State::Running) {
        return;
    }

    auto candidate = active_;
    ++candidate.x;
    if (canPlace(candidate)) {
        active_ = candidate;
    }
}

void Game::softDrop() {
    if (state_ != State::Running) {
        return;
    }

    auto candidate = active_;
    ++candidate.y;
    if (canPlace(candidate)) {
        active_ = candidate;
        ++score_;
        dropAccumulator_ = Seconds::zero();
    } else {
        lockActivePiece();
    }
}

void Game::hardDrop() {
    if (state_ != State::Running) {
        return;
    }

    int distance = 0;
    auto candidate = active_;
    while (true) {
        auto next = candidate;
        ++next.y;
        if (!canPlace(next)) {
            break;
        }
        candidate = next;
        ++distance;
    }

    active_ = candidate;
    score_ += distance * 2;
    lockActivePiece();
}

void Game::rotateClockwise() {
    if (state_ != State::Running) {
        return;
    }

    auto candidate = active_;
    candidate.rotation = (candidate.rotation + 1) % 4;

    for (const int offsetX : kWallKickOffsets) {
        auto kicked = candidate;
        kicked.x += offsetX;
        if (canPlace(kicked)) {
            active_ = kicked;
            return;
        }
    }
}

void Game::rotateCounterClockwise() {
    if (state_ != State::Running) {
        return;
    }

    auto candidate = active_;
    candidate.rotation = (candidate.rotation + 3) % 4;

    for (const int offsetX : kWallKickOffsets) {
        auto kicked = candidate;
        kicked.x += offsetX;
        if (canPlace(kicked)) {
            active_ = kicked;
            return;
        }
    }
}

void Game::togglePause() {
    switch (state_) {
        case State::Running:
            state_ = State::Paused;
            break;
        case State::Paused:
            state_ = State::Running;
            dropAccumulator_ = Seconds::zero();
            break;
        case State::GameOver:
            break;
    }
}

Game::Piece Game::ghostPiece() const {
    auto ghost = active_;
    while (true) {
        auto next = ghost;
        ++next.y;
        if (!canPlace(next)) {
            break;
        }
        ghost = next;
    }
    return ghost;
}

std::array<TetrominoType, Game::PreviewCount> Game::nextPreview() const {
    std::array<TetrominoType, PreviewCount> result{};
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = queue_.at(i);
    }
    return result;
}

std::string Game::titleText() const {
    std::ostringstream title;
    title << "Tetris C++23 | Score: " << score_
          << " | Lines: " << lines_
          << " | Level: " << level_;

    switch (state_) {
        case State::Running:
            break;
        case State::Paused:
            title << " | PAUSED";
            break;
        case State::GameOver:
            title << " | GAME OVER - press R";
            break;
    }

    return title.str();
}

bool Game::isCellInside(int x, int y) const noexcept {
    return x >= 0 && x < BoardWidth && y >= 0 && y < BoardHeight;
}

bool Game::isCellOccupied(int x, int y) const noexcept {
    return board_[boardIndex(x, y)] != 0;
}

bool Game::canPlace(const Piece& piece) const noexcept {
    for (const auto& [localX, localY] : shape(piece.type, piece.rotation)) {
        const int x = piece.x + localX;
        const int y = piece.y + localY;

        if (!isCellInside(x, y) || isCellOccupied(x, y)) {
            return false;
        }
    }
    return true;
}

void Game::moveDownByTimer() {
    auto candidate = active_;
    ++candidate.y;
    if (canPlace(candidate)) {
        active_ = candidate;
    } else {
        lockActivePiece();
    }
}

void Game::lockActivePiece() {
    for (const auto& [localX, localY] : shape(active_.type, active_.rotation)) {
        const int x = active_.x + localX;
        const int y = active_.y + localY;
        if (isCellInside(x, y)) {
            board_[boardIndex(x, y)] = std::to_underlying(active_.type) + 1;
        }
    }

    const int cleared = clearCompletedLines();
    addScoreForClearedLines(cleared);
    spawnNextPiece();
    dropAccumulator_ = Seconds::zero();
}

int Game::clearCompletedLines() {
    int cleared = 0;

    for (int y = BoardHeight - 1; y >= 0; --y) {
        bool full = true;
        for (int x = 0; x < BoardWidth; ++x) {
            if (board_[boardIndex(x, y)] == 0) {
                full = false;
                break;
            }
        }

        if (!full) {
            continue;
        }

        ++cleared;
        for (int row = y; row > 0; --row) {
            for (int x = 0; x < BoardWidth; ++x) {
                board_[boardIndex(x, row)] = board_[boardIndex(x, row - 1)];
            }
        }

        for (int x = 0; x < BoardWidth; ++x) {
            board_[boardIndex(x, 0)] = 0;
        }

        ++y; // Re-check the same row after it received data from above.
    }

    return cleared;
}

void Game::spawnNextPiece() {
    fillQueue();
    active_ = Piece{queue_.front(), 0, 3, 0};
    queue_.pop_front();
    fillQueue();

    if (!canPlace(active_)) {
        state_ = State::GameOver;
    }
}

void Game::fillQueue() {
    while (queue_.size() < static_cast<std::size_t>(PreviewCount + 1)) {
        queue_.push_back(takeFromBag());
    }
}

TetrominoType Game::takeFromBag() {
    if (bag_.empty()) {
        bag_.reserve(tetrominoCount());
        for (int i = 0; i < std::to_underlying(TetrominoType::Count); ++i) {
            bag_.push_back(static_cast<TetrominoType>(i));
        }
        std::ranges::shuffle(bag_, rng_);
    }

    const auto result = bag_.back();
    bag_.pop_back();
    return result;
}

void Game::addScoreForClearedLines(int count) {
    if (count <= 0) {
        return;
    }

    score_ += kLineScores.at(static_cast<std::size_t>(count)) * level_;
    lines_ += count;
    level_ = (lines_ / 10) + 1;
}

Game::Seconds Game::dropInterval() const noexcept {
    const auto levelPenalty = kLevelDropStep * static_cast<double>(level_ - 1);
    return std::max(kMinDropInterval, kInitialDropInterval - levelPenalty);
}

} // namespace tetris
