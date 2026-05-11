#include "Game.h"

#include <algorithm>
#include <sstream>

namespace tetris {

Game::Game()
    : board_(BoardWidth * BoardHeight, 0),
      rng_(std::random_device{}()) {
    reset();
}

void Game::reset() {
    std::fill(board_.begin(), board_.end(), 0);
    queue_.clear();
    bag_.clear();
    dropAccumulator_ = 0.0;
    score_ = 0;
    lines_ = 0;
    level_ = 1;
    state_ = State::Running;
    fillQueue();
    spawnNextPiece();
}

void Game::update(double deltaSeconds) {
    if (state_ != State::Running) {
        return;
    }

    dropAccumulator_ += deltaSeconds;
    while (dropAccumulator_ >= dropIntervalSeconds() && state_ == State::Running) {
        moveDownByTimer();
        dropAccumulator_ -= dropIntervalSeconds();
    }
}

void Game::moveLeft() {
    if (state_ != State::Running) return;
    Piece candidate = active_;
    --candidate.x;
    if (canPlace(candidate)) active_ = candidate;
}

void Game::moveRight() {
    if (state_ != State::Running) return;
    Piece candidate = active_;
    ++candidate.x;
    if (canPlace(candidate)) active_ = candidate;
}

void Game::softDrop() {
    if (state_ != State::Running) return;
    Piece candidate = active_;
    ++candidate.y;
    if (canPlace(candidate)) {
        active_ = candidate;
        score_ += 1;
        dropAccumulator_ = 0.0;
    } else {
        lockActivePiece();
    }
}

void Game::hardDrop() {
    if (state_ != State::Running) return;

    int distance = 0;
    Piece candidate = active_;
    while (true) {
        Piece next = candidate;
        ++next.y;
        if (!canPlace(next)) break;
        candidate = next;
        ++distance;
    }

    active_ = candidate;
    score_ += distance * 2;
    lockActivePiece();
}

void Game::rotateClockwise() {
    if (state_ != State::Running) return;

    Piece candidate = active_;
    candidate.rotation = (candidate.rotation + 1) % 4;

    // Small wall-kick set. It is intentionally simple, predictable and safe.
    for (const int offsetX : {0, -1, 1, -2, 2}) {
        Piece kicked = candidate;
        kicked.x += offsetX;
        if (canPlace(kicked)) {
            active_ = kicked;
            return;
        }
    }
}

void Game::rotateCounterClockwise() {
    if (state_ != State::Running) return;

    Piece candidate = active_;
    candidate.rotation = (candidate.rotation + 3) % 4;

    for (const int offsetX : {0, -1, 1, -2, 2}) {
        Piece kicked = candidate;
        kicked.x += offsetX;
        if (canPlace(kicked)) {
            active_ = kicked;
            return;
        }
    }
}

void Game::togglePause() {
    if (state_ == State::Running) {
        state_ = State::Paused;
    } else if (state_ == State::Paused) {
        state_ = State::Running;
        dropAccumulator_ = 0.0;
    }
}

Game::Piece Game::ghostPiece() const {
    Piece ghost = active_;
    while (true) {
        Piece next = ghost;
        ++next.y;
        if (!canPlace(next)) break;
        ghost = next;
    }
    return ghost;
}

std::array<TetrominoType, Game::PreviewCount> Game::nextPreview() const {
    std::array<TetrominoType, PreviewCount> result{};
    for (int i = 0; i < PreviewCount; ++i) {
        result[static_cast<std::size_t>(i)] = queue_.at(static_cast<std::size_t>(i));
    }
    return result;
}

std::string Game::titleText() const {
    std::ostringstream title;
    title << "Tetris | Score: " << score_
          << " | Lines: " << lines_
          << " | Level: " << level_;

    if (state_ == State::Paused) {
        title << " | PAUSED";
    } else if (state_ == State::GameOver) {
        title << " | GAME OVER - press R";
    }

    return title.str();
}

bool Game::isCellInside(int x, int y) const noexcept {
    return x >= 0 && x < BoardWidth && y >= 0 && y < BoardHeight;
}

bool Game::isCellOccupied(int x, int y) const noexcept {
    return board_[static_cast<std::size_t>(y * BoardWidth + x)] != 0;
}

bool Game::canPlace(const Piece& piece) const noexcept {
    for (const Cell& cell : shape(piece.type, piece.rotation)) {
        const int x = piece.x + cell.x;
        const int y = piece.y + cell.y;

        if (!isCellInside(x, y)) {
            return false;
        }

        if (isCellOccupied(x, y)) {
            return false;
        }
    }
    return true;
}

void Game::moveDownByTimer() {
    Piece candidate = active_;
    ++candidate.y;
    if (canPlace(candidate)) {
        active_ = candidate;
    } else {
        lockActivePiece();
    }
}

void Game::lockActivePiece() {
    for (const Cell& cell : shape(active_.type, active_.rotation)) {
        const int x = active_.x + cell.x;
        const int y = active_.y + cell.y;
        if (isCellInside(x, y)) {
            board_[static_cast<std::size_t>(y * BoardWidth + x)] = static_cast<int>(active_.type) + 1;
        }
    }

    const int cleared = clearCompletedLines();
    addScoreForClearedLines(cleared);
    spawnNextPiece();
    dropAccumulator_ = 0.0;
}

int Game::clearCompletedLines() {
    int cleared = 0;

    for (int y = BoardHeight - 1; y >= 0; --y) {
        bool full = true;
        for (int x = 0; x < BoardWidth; ++x) {
            if (board_[static_cast<std::size_t>(y * BoardWidth + x)] == 0) {
                full = false;
                break;
            }
        }

        if (!full) continue;

        ++cleared;
        for (int row = y; row > 0; --row) {
            for (int x = 0; x < BoardWidth; ++x) {
                board_[static_cast<std::size_t>(row * BoardWidth + x)] =
                    board_[static_cast<std::size_t>((row - 1) * BoardWidth + x)];
            }
        }

        for (int x = 0; x < BoardWidth; ++x) {
            board_[static_cast<std::size_t>(x)] = 0;
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
        for (int i = 0; i < static_cast<int>(TetrominoType::Count); ++i) {
            bag_.push_back(static_cast<TetrominoType>(i));
        }
        std::shuffle(bag_.begin(), bag_.end(), rng_);
    }

    TetrominoType result = bag_.back();
    bag_.pop_back();
    return result;
}

void Game::addScoreForClearedLines(int count) {
    if (count <= 0) return;

    static constexpr std::array<int, 5> kLineScores = {0, 100, 300, 500, 800};
    score_ += kLineScores[static_cast<std::size_t>(count)] * level_;
    lines_ += count;
    level_ = (lines_ / 10) + 1;
}

double Game::dropIntervalSeconds() const noexcept {
    const double interval = 0.70 - static_cast<double>(level_ - 1) * 0.055;
    return std::max(0.08, interval);
}

} // namespace tetris
