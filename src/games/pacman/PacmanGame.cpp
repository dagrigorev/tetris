#include "games/pacman/PacmanGame.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace games::pacman {
namespace {

[[nodiscard]] auto distanceSquared(const Actor& a, const Actor& b) -> double {
    const auto dx = a.x - b.x;
    const auto dy = a.y - b.y;
    return dx * dx + dy * dy;
}

} // namespace

PacmanGame::PacmanGame(gamecore::EventBus& events) : events_(events) {
    restart();
}

auto PacmanGame::name() const -> std::string_view {
    return "Pacman";
}

void PacmanGame::restart() {
    score_ = 0;
    lives_ = 3;
    quit_ = false;
    animationTime_ = 0.0;
    phase_ = PacmanPhase::Ready;
    resetPellets();
    resetRound();
}

void PacmanGame::handleInput(const gamecore::InputFrame& input) {
    using enum gamecore::InputCommand;

    if (input.isPressed(Quit)) {
        quit_ = true;
        return;
    }

    if (input.isPressed(Restart)) {
        restart();
        return;
    }

    if (input.isPressed(Pause) && (phase_ == PacmanPhase::Playing || phase_ == PacmanPhase::Paused)) {
        phase_ = phase_ == PacmanPhase::Paused ? PacmanPhase::Playing : PacmanPhase::Paused;
        return;
    }

    auto requested = Direction::None;
    if (input.isDown(MoveLeft)) {
        requested = Direction::Left;
    } else if (input.isDown(MoveRight)) {
        requested = Direction::Right;
    } else if (input.isDown(MoveUp)) {
        requested = Direction::Up;
    } else if (input.isDown(MoveDown) || input.isDown(SoftDrop)) {
        requested = Direction::Down;
    }

    if (requested != Direction::None) {
        pacman_.requested = requested;

        if (phase_ == PacmanPhase::Ready) {
            phase_ = PacmanPhase::Playing;
        }

        // Apply an initial or buffered turn immediately when Pacman is already
        // aligned with the grid. This makes the first movement key visibly start
        // the game on the same frame instead of waiting for another update cycle.
        if (centeredOnTile(pacman_.x, MazeOffsetX) && centeredOnTile(pacman_.y, MazeOffsetY) &&
            canMoveFrom(pacman_.x, pacman_.y, pacman_.requested)) {
            pacman_.direction = pacman_.requested;
        }
    }

    if ((input.isPressed(Select) || input.isPressed(HardDrop)) && phase_ == PacmanPhase::Ready) {
        phase_ = PacmanPhase::Playing;
    }
}

void PacmanGame::update(const gamecore::Seconds deltaTime) {
    const auto dt = std::clamp(deltaTime.count(), 0.0, 0.035);

    animationTime_ += dt;

    if (phase_ != PacmanPhase::Playing) {
        return;
    }

    updatePacman(dt);
    collectPellet();
    updateGhosts(dt);
    resolveGhostCollision();

    if (remainingPellets_ <= 0) {
        phase_ = PacmanPhase::Won;
        events_.publish({"pacman.level_completed", score_});
    }
}

void PacmanGame::render(gamecore::IRenderer2D& renderer) const {
    renderer.beginFrame({2, 3, 12, 255});

    renderer.fillRect({24, 28, ScreenWidth - 48, ScreenHeight - 56}, {5, 7, 22, 255});
    renderer.drawRect({MazeOffsetX - 8, MazeOffsetY - 8, MazeWidth * TileSize + 16, MazeHeight * TileSize + 16}, {50, 80, 180, 255});

    const auto& rows = mazeRows();
    for (int row = 0; row < MazeHeight; ++row) {
        for (int col = 0; col < MazeWidth; ++col) {
            const auto x = MazeOffsetX + col * TileSize;
            const auto y = MazeOffsetY + row * TileSize;
            if (rows[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] == '#') {
                renderer.fillRect({x, y, TileSize, TileSize}, {22, 42, 142, 255});
                renderer.drawRect({x, y, TileSize, TileSize}, {75, 110, 235, 255});
                continue;
            }

            const auto pellet = pellets_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            if (pellet == '.') {
                renderer.fillRect({x + 10, y + 10, 4, 4}, {245, 220, 165, 255});
            } else if (pellet == 'o') {
                renderer.fillRect({x + 7, y + 7, 10, 10}, {255, 235, 175, 255});
            }
        }
    }

    int ghostIndex = 0;
    for (const auto& ghost : ghosts_) {
        const auto bob = static_cast<int>(std::lround(std::sin(animationTime_ * 9.0 + ghostIndex) * 1.4));
        const auto rect = actorRect(ghost.actor, 22);
        const gamecore::Vec2i center{rect.x + rect.width / 2, rect.y + rect.height / 2 + bob};
        const auto bodyTop = rect.y + bob;

        renderer.fillCircle({center.x, bodyTop + 11}, 11, ghost.color);
        renderer.fillRect({rect.x, bodyTop + 11, rect.width, 10}, ghost.color);
        const auto wave = static_cast<int>(std::lround(std::sin(animationTime_ * 14.0 + ghostIndex) * 2.0));
        renderer.fillCircle({rect.x + 4, bodyTop + 21 + wave}, 4, ghost.color);
        renderer.fillCircle({rect.x + 11, bodyTop + 21 - wave}, 4, ghost.color);
        renderer.fillCircle({rect.x + 18, bodyTop + 21 + wave}, 4, ghost.color);
        renderer.drawLine({rect.x + 1, bodyTop + 13}, {rect.x + 21, bodyTop + 13}, {255, 255, 255, 45});

        const auto eyeShift = directionVector(ghost.actor.direction);
        renderer.fillCircle({rect.x + 7, bodyTop + 10}, 4, {245, 250, 255, 255});
        renderer.fillCircle({rect.x + 15, bodyTop + 10}, 4, {245, 250, 255, 255});
        renderer.fillCircle({rect.x + 8 + eyeShift.x, bodyTop + 10 + eyeShift.y}, 2, {20, 35, 115, 255});
        renderer.fillCircle({rect.x + 16 + eyeShift.x, bodyTop + 10 + eyeShift.y}, 2, {20, 35, 115, 255});
        ++ghostIndex;
    }

    const auto pac = actorRect(pacman_, 22);
    const gamecore::Vec2i pacCenter{pac.x + pac.width / 2, pac.y + pac.height / 2};
    renderer.fillCircle(pacCenter, 11, {255, 220, 55, 255});
    renderer.drawCircle(pacCenter, 11, {255, 245, 145, 220});

    const auto mouthColor = gamecore::Color{5, 7, 22, 255};
    const auto moving = pacman_.direction != Direction::None && phase_ == PacmanPhase::Playing;
    const auto mouth = moving ? (3 + static_cast<int>(std::lround((std::sin(animationTime_ * 18.0) + 1.0) * 4.0))) : 7;
    if (pacman_.direction == Direction::Right || pacman_.direction == Direction::None) {
        renderer.fillTriangle(pacCenter, {pacCenter.x + 13, pacCenter.y - mouth}, {pacCenter.x + 13, pacCenter.y + mouth}, mouthColor);
    } else if (pacman_.direction == Direction::Left) {
        renderer.fillTriangle(pacCenter, {pacCenter.x - 13, pacCenter.y - mouth}, {pacCenter.x - 13, pacCenter.y + mouth}, mouthColor);
    } else if (pacman_.direction == Direction::Up) {
        renderer.fillTriangle(pacCenter, {pacCenter.x - mouth, pacCenter.y - 13}, {pacCenter.x + mouth, pacCenter.y - 13}, mouthColor);
    } else {
        renderer.fillTriangle(pacCenter, {pacCenter.x - mouth, pacCenter.y + 13}, {pacCenter.x + mouth, pacCenter.y + 13}, mouthColor);
    }

    renderer.drawText({48, 10}, "PACMAN", {235, 244, 255, 255}, 2);
    renderer.drawText({212, 10}, std::string{"SCORE "} + std::to_string(score_), {190, 210, 240, 255}, 2);
    renderer.drawText({538, 10}, std::string{"LIVES "} + std::to_string(lives_), {190, 210, 240, 255}, 2);
    renderer.drawText({64, 616}, "W A S D OR ARROWS MOVE   P PAUSE   R RESTART   ESC MENU", {132, 150, 185, 255}, 1);

    if (phase_ == PacmanPhase::Ready) {
        renderer.fillRect({188, 270, 384, 96}, {0, 0, 0, 220});
        renderer.drawRect({188, 270, 384, 96}, {125, 190, 255, 255});
        renderer.drawText({260, 294}, "PACMAN", {255, 230, 70, 255}, 4);
        renderer.drawText({236, 344}, "MOVE TO START", {160, 205, 255, 255}, 1);
    } else if (phase_ == PacmanPhase::Paused) {
        renderer.fillRect({250, 282, 260, 74}, {0, 0, 0, 215});
        renderer.drawRect({250, 282, 260, 74}, {245, 250, 255, 255});
        renderer.drawText({302, 306}, "PAUSED", {245, 250, 255, 255}, 3);
    } else if (phase_ == PacmanPhase::Won) {
        renderer.fillRect({196, 256, 368, 122}, {0, 0, 0, 225});
        renderer.drawRect({196, 256, 368, 122}, {120, 255, 180, 255});
        renderer.drawText({264, 284}, "YOU WIN", {150, 255, 190, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    } else if (phase_ == PacmanPhase::GameOver) {
        renderer.fillRect({178, 256, 404, 122}, {0, 0, 0, 225});
        renderer.drawRect({178, 256, 404, 122}, {255, 90, 90, 255});
        renderer.drawText({238, 284}, "GAME OVER", {255, 120, 120, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    }

    renderer.endFrame();
}

auto PacmanGame::wantsToQuit() const -> bool {
    return quit_;
}

void PacmanGame::resetRound() {
    // Pacman starts idle. The first movement key both starts the round and
    // selects the initial direction. This is less confusing than silently
    // moving left after pressing Enter/Space.
    pacman_ = Actor{.x = tileCenterX(9), .y = tileCenterY(15), .direction = Direction::None, .requested = Direction::None, .speed = 132.0};
    ghosts_.clear();
    ghosts_.push_back(Ghost{.actor = Actor{.x = tileCenterX(9), .y = tileCenterY(9), .direction = Direction::Left, .requested = Direction::Left, .speed = 92.0}, .color = {255, 85, 85, 255}});
    ghosts_.push_back(Ghost{.actor = Actor{.x = tileCenterX(8), .y = tileCenterY(10), .direction = Direction::Right, .requested = Direction::Right, .speed = 88.0}, .color = {255, 150, 220, 255}});
    ghosts_.push_back(Ghost{.actor = Actor{.x = tileCenterX(10), .y = tileCenterY(10), .direction = Direction::Left, .requested = Direction::Left, .speed = 84.0}, .color = {80, 220, 255, 255}});
}

void PacmanGame::resetPellets() {
    remainingPellets_ = 0;
    const auto& rows = mazeRows();
    for (int row = 0; row < MazeHeight; ++row) {
        pellets_[static_cast<std::size_t>(row)] = std::string{rows[static_cast<std::size_t>(row)]};
        for (int col = 0; col < MazeWidth; ++col) {
            auto& cell = pellets_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            if (cell == 'P' || cell == 'G' || cell == ' ') {
                cell = ' ';
            }
            if (cell == '.' || cell == 'o') {
                ++remainingPellets_;
            }
        }
    }
}

void PacmanGame::updatePacman(const double dt) {
    if (centeredOnTile(pacman_.x, MazeOffsetX) && centeredOnTile(pacman_.y, MazeOffsetY)) {
        const auto col = currentCol(pacman_.x);
        const auto row = currentRow(pacman_.y);
        pacman_.x = tileCenterX(col);
        pacman_.y = tileCenterY(row);

        if (canMoveFrom(pacman_.x, pacman_.y, pacman_.requested)) {
            pacman_.direction = pacman_.requested;
        } else if (!canMoveFrom(pacman_.x, pacman_.y, pacman_.direction)) {
            pacman_.direction = Direction::None;
        }
    }

    moveActorAlongGrid(pacman_, dt);
}

void PacmanGame::updateGhosts(const double dt) {
    for (auto& ghost : ghosts_) {
        auto& actor = ghost.actor;

        if (centeredOnTile(actor.x, MazeOffsetX) && centeredOnTile(actor.y, MazeOffsetY)) {
            const auto col = currentCol(actor.x);
            const auto row = currentRow(actor.y);
            actor.x = tileCenterX(col);
            actor.y = tileCenterY(row);

            std::vector<Direction> choices;
            for (const auto direction : {Direction::Left, Direction::Right, Direction::Up, Direction::Down}) {
                if (direction != opposite(actor.direction) && canMoveFrom(actor.x, actor.y, direction)) {
                    choices.push_back(direction);
                }
            }
            if (choices.empty() && canMoveFrom(actor.x, actor.y, opposite(actor.direction))) {
                choices.push_back(opposite(actor.direction));
            }

            if (!choices.empty()) {
                auto best = choices.front();
                auto bestScore = 1'000'000.0;
                for (const auto choice : choices) {
                    const auto vec = directionVector(choice);
                    const auto nx = actor.x + static_cast<double>(vec.x) * TileSize;
                    const auto ny = actor.y + static_cast<double>(vec.y) * TileSize;
                    const auto dx = nx - pacman_.x;
                    const auto dy = ny - pacman_.y;
                    auto score = dx * dx + dy * dy;
                    if (ghost.color.b > ghost.color.r) {
                        score = -score;
                    }
                    if (score < bestScore) {
                        bestScore = score;
                        best = choice;
                    }
                }
                actor.direction = best;
            }
        }

        moveActorAlongGrid(actor, dt);
    }
}

void PacmanGame::moveActorAlongGrid(Actor& actor, const double dt) {
    if (actor.direction == Direction::None) {
        return;
    }

    if (centeredOnTile(actor.x, MazeOffsetX) && centeredOnTile(actor.y, MazeOffsetY) &&
        !canMoveFrom(actor.x, actor.y, actor.direction)) {
        actor.x = tileCenterX(currentCol(actor.x));
        actor.y = tileCenterY(currentRow(actor.y));
        actor.direction = Direction::None;
        return;
    }

    const auto vec = directionVector(actor.direction);
    const auto step = actor.speed * dt;

    if (vec.x != 0) {
        const auto localX = (actor.x - static_cast<double>(MazeOffsetX + TileSize / 2)) / static_cast<double>(TileSize);
        const auto targetCol = vec.x > 0
            ? static_cast<int>(std::floor(localX + 0.000001)) + 1
            : static_cast<int>(std::ceil(localX - 0.000001)) - 1;
        const auto targetX = tileCenterX(targetCol);
        auto nextX = actor.x + static_cast<double>(vec.x) * step;

        if ((vec.x > 0 && nextX >= targetX) || (vec.x < 0 && nextX <= targetX)) {
            nextX = targetX;
        }

        actor.x = nextX;
        actor.y = tileCenterY(currentRow(actor.y));
        return;
    }

    if (vec.y != 0) {
        const auto localY = (actor.y - static_cast<double>(MazeOffsetY + TileSize / 2)) / static_cast<double>(TileSize);
        const auto targetRow = vec.y > 0
            ? static_cast<int>(std::floor(localY + 0.000001)) + 1
            : static_cast<int>(std::ceil(localY - 0.000001)) - 1;
        const auto targetY = tileCenterY(targetRow);
        auto nextY = actor.y + static_cast<double>(vec.y) * step;

        if ((vec.y > 0 && nextY >= targetY) || (vec.y < 0 && nextY <= targetY)) {
            nextY = targetY;
        }

        actor.y = nextY;
        actor.x = tileCenterX(currentCol(actor.x));
    }
}

void PacmanGame::collectPellet() {
    const auto col = currentCol(pacman_.x);
    const auto row = currentRow(pacman_.y);
    if (row < 0 || row >= MazeHeight || col < 0 || col >= MazeWidth) {
        return;
    }

    auto& cell = pellets_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
    if (cell == '.') {
        cell = ' ';
        score_ += 10;
        --remainingPellets_;
    } else if (cell == 'o') {
        cell = ' ';
        score_ += 50;
        --remainingPellets_;
    }
}

void PacmanGame::resolveGhostCollision() {
    for (const auto& ghost : ghosts_) {
        if (distanceSquared(pacman_, ghost.actor) < 18.0 * 18.0) {
            loseLife();
            return;
        }
    }
}

void PacmanGame::loseLife() {
    --lives_;
    if (lives_ <= 0) {
        phase_ = PacmanPhase::GameOver;
        events_.publish({"pacman.game_over", score_});
        return;
    }
    phase_ = PacmanPhase::Ready;
    resetRound();
}

auto PacmanGame::canMoveFrom(const double x, const double y, const Direction direction) const -> bool {
    if (direction == Direction::None) {
        return false;
    }
    const auto col = currentCol(x);
    const auto row = currentRow(y);
    const auto vec = directionVector(direction);
    return !isWallCell(col + vec.x, row + vec.y);
}

auto PacmanGame::isWallCell(const int col, const int row) const -> bool {
    if (row < 0 || row >= MazeHeight || col < 0 || col >= MazeWidth) {
        return true;
    }
    return mazeRows()[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] == '#';
}

auto PacmanGame::currentCol(const double x) const -> int {
    return static_cast<int>(std::lround((x - MazeOffsetX - TileSize / 2.0) / TileSize));
}

auto PacmanGame::currentRow(const double y) const -> int {
    return static_cast<int>(std::lround((y - MazeOffsetY - TileSize / 2.0) / TileSize));
}

auto PacmanGame::centeredOnTile(const double value, const int offset) const -> bool {
    const auto local = std::fmod(value - static_cast<double>(offset + TileSize / 2), static_cast<double>(TileSize));
    constexpr auto tolerancePixels = 1.5;
    return std::abs(local) <= tolerancePixels || std::abs(std::abs(local) - static_cast<double>(TileSize)) <= tolerancePixels;
}

auto PacmanGame::tileCenterX(const int col) const -> double {
    return static_cast<double>(MazeOffsetX + col * TileSize + TileSize / 2);
}

auto PacmanGame::tileCenterY(const int row) const -> double {
    return static_cast<double>(MazeOffsetY + row * TileSize + TileSize / 2);
}

auto PacmanGame::directionVector(const Direction direction) -> gamecore::Vec2i {
    switch (direction) {
        case Direction::Left: return {-1, 0};
        case Direction::Right: return {1, 0};
        case Direction::Up: return {0, -1};
        case Direction::Down: return {0, 1};
        case Direction::None: return {0, 0};
    }
    return {0, 0};
}

auto PacmanGame::actorRect(const Actor& actor, const int size) -> gamecore::Recti {
    return {
        static_cast<int>(std::lround(actor.x - static_cast<double>(size) / 2.0)),
        static_cast<int>(std::lround(actor.y - static_cast<double>(size) / 2.0)),
        size,
        size
    };
}

auto PacmanGame::opposite(const Direction direction) -> Direction {
    switch (direction) {
        case Direction::Left: return Direction::Right;
        case Direction::Right: return Direction::Left;
        case Direction::Up: return Direction::Down;
        case Direction::Down: return Direction::Up;
        case Direction::None: return Direction::None;
    }
    return Direction::None;
}

auto PacmanGame::mazeRows() -> const MazeRows& {
    static constexpr MazeRows rows{
        "###################",
        "#o.......#.......o#",
        "#.###.###.#.###.###",
        "#.................#",
        "#.###.#.#####.#.###",
        "#.....#...#...#...#",
        "#####.### # ###.###",
        "    #.#       #.#  ",
        "#####.# ## ## #.###",
        "     .  #GG#  .    ",
        "#####.# ##### #.###",
        "    #.#       #.#  ",
        "#####.# ##### #.###",
        "#........#........#",
        "#.###.###.#.###.###",
        "#o..#.....P.....#o#",
        "###.#.#.#####.#.#.#",
        "#.....#...#...#...#",
        "#.#######.#.#######",
        "#.................#",
        "###################"
    };
    return rows;
}

} // namespace games::pacman
