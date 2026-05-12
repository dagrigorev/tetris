#include "games/battle_city/BattleCityGame.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace games::battle_city {
namespace {

[[nodiscard]] auto tileBlocksTank(const Tile tile) -> bool {
    return tile == Tile::Brick || tile == Tile::Steel || tile == Tile::Water || tile == Tile::Base || tile == Tile::DestroyedBase;
}

[[nodiscard]] auto tileBlocksBullet(const Tile tile) -> bool {
    return tile == Tile::Brick || tile == Tile::Steel || tile == Tile::Base || tile == Tile::DestroyedBase;
}

[[nodiscard]] auto baseColor(const bool player) -> gamecore::Color {
    return player ? gamecore::Color{238, 204, 82, 255} : gamecore::Color{92, 178, 100, 255};
}

} // namespace

BattleCityGame::BattleCityGame(gamecore::EventBus& events) : events_(events) { restart(); }

auto BattleCityGame::name() const -> std::string_view { return "Battle City"; }

void BattleCityGame::restart() {
    score_ = 0;
    lives_ = 3;
    stage_ = 1;
    enemiesRemaining_ = 16;
    spawnCursor_ = 0;
    spawnTimer_ = 0.2;
    messageTimer_ = 0.0;
    animationTime_ = 0.0;
    requestedDirection_ = Direction::Up;
    hasMoveInput_ = false;
    fireRequested_ = false;
    quit_ = false;
    bullets_.clear();
    explosions_.clear();
    enemies_.clear();
    buildStage();
    player_ = Tank{.position = {BoardLeft + 9.0 * TileSize + 1.0, BoardTop + 20.0 * TileSize + 1.0}, .direction = Direction::Up, .speed = 128.0, .player = true, .alive = true, .fireCooldown = 0.0, .aiTimer = 0.0, .hp = 1};
    phase_ = BattlePhase::Ready;
}

void BattleCityGame::handleInput(const gamecore::InputFrame& input) {
    using enum gamecore::InputCommand;

    if (input.isPressed(Quit)) { quit_ = true; return; }
    if (input.isPressed(Restart)) { restart(); return; }
    if (input.isPressed(Pause) && (phase_ == BattlePhase::Playing || phase_ == BattlePhase::Paused)) {
        phase_ = phase_ == BattlePhase::Paused ? BattlePhase::Playing : BattlePhase::Paused;
        return;
    }

    hasMoveInput_ = false;
    if (input.isDown(MoveUp)) { requestedDirection_ = Direction::Up; hasMoveInput_ = true; }
    else if (input.isDown(MoveDown)) { requestedDirection_ = Direction::Down; hasMoveInput_ = true; }
    else if (input.isDown(MoveLeft)) { requestedDirection_ = Direction::Left; hasMoveInput_ = true; }
    else if (input.isDown(MoveRight)) { requestedDirection_ = Direction::Right; hasMoveInput_ = true; }

    fireRequested_ = input.isPressed(Select) || input.isPressed(HardDrop);

    if (phase_ == BattlePhase::Ready && (hasMoveInput_ || fireRequested_)) {
        phase_ = BattlePhase::Playing;
    }
    if (phase_ == BattlePhase::GameOver && input.isPressed(Select)) {
        restart();
    }
}

void BattleCityGame::update(const gamecore::Seconds deltaTime) {
    const auto dt = std::clamp(deltaTime.count(), 0.0, 0.035);
    animationTime_ += dt;
    for (auto& explosion : explosions_) { explosion.age += dt; }
    std::erase_if(explosions_, [](const Explosion& explosion) { return explosion.age >= explosion.duration; });

    if (phase_ == BattlePhase::StageCleared) {
        messageTimer_ -= dt;
        if (messageTimer_ <= 0.0) {
            ++stage_;
            enemiesRemaining_ = 16 + stage_ * 2;
            spawnCursor_ = 0;
            spawnTimer_ = 0.25;
            bullets_.clear();
            explosions_.clear();
            enemies_.clear();
            buildStage();
            player_.position = {BoardLeft + 9.0 * TileSize + 1.0, BoardTop + 20.0 * TileSize + 1.0};
            player_.direction = Direction::Up;
            phase_ = BattlePhase::Playing;
        }
        return;
    }

    if (phase_ != BattlePhase::Playing) { return; }

    player_.fireCooldown = std::max(0.0, player_.fireCooldown - dt);
    updatePlayer(dt);
    updateEnemies(dt);
    updateBullets(dt);

    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.0 && enemiesRemaining_ > 0 && enemies_.size() < 5U) {
        spawnEnemy();
        spawnTimer_ = std::max(1.0, 2.6 - static_cast<double>(stage_) * 0.12);
    }

    if (enemiesRemaining_ <= 0 && enemies_.empty()) {
        clearStage();
    }
}

void BattleCityGame::render(gamecore::IRenderer2D& renderer) const {
    renderer.beginFrame({9, 10, 14, 255});

    renderer.fillRect({28, 28, ScreenWidth - 56, ScreenHeight - 56}, {18, 19, 24, 255});
    renderer.fillRect({BoardLeft - 6, BoardTop - 6, BoardCols * TileSize + 12, BoardRows * TileSize + 12}, {37, 38, 43, 255});
    renderer.fillRect({BoardLeft, BoardTop, BoardCols * TileSize, BoardRows * TileSize}, {11, 12, 16, 255});

    for (int row = 0; row < BoardRows; ++row) {
        for (int col = 0; col < BoardCols; ++col) {
            const auto tile = tileAt(col, row);
            if (tile != Tile::Empty) { drawTile(renderer, col, row, tile); }
        }
    }

    for (const auto& bullet : bullets_) {
        if (!bullet.active) { continue; }
        const auto v = directionVector(bullet.direction);
        const gamecore::Vec2i center{static_cast<int>(std::lround(bullet.position.x)), static_cast<int>(std::lround(bullet.position.y))};
        renderer.drawLine({center.x - static_cast<int>(v.x * 8.0), center.y - static_cast<int>(v.y * 8.0)}, center, bullet.fromPlayer ? gamecore::Color{255, 246, 170, 180} : gamecore::Color{255, 118, 90, 180});
        renderer.fillCircle(center, BulletSize / 2, bullet.fromPlayer ? gamecore::Color{255, 244, 146, 255} : gamecore::Color{255, 92, 82, 255});
        renderer.fillCircle(center, 1, {255, 255, 255, 255});
    }

    for (const auto& explosion : explosions_) {
        drawExplosion(renderer, explosion);
    }

    for (const auto& enemy : enemies_) {
        if (enemy.alive) { drawTank(renderer, enemy); }
    }
    if (player_.alive) { drawTank(renderer, player_); }

    renderer.drawText({44, 8}, "BATTLE CITY", {235, 244, 255, 255}, 2);
    renderer.drawText({238, 8}, std::string{"SCORE "} + std::to_string(score_), {196, 215, 245, 255}, 2);
    renderer.drawText({436, 8}, std::string{"STAGE "} + std::to_string(stage_), {196, 215, 245, 255}, 2);
    renderer.drawText({596, 8}, std::string{"LIVES "} + std::to_string(lives_), {196, 215, 245, 255}, 2);
    renderer.drawText({54, 616}, "WASD/ARROWS MOVE   SPACE FIRE/START   P PAUSE   R RESTART   ESC MENU", {142, 154, 184, 255}, 1);

    if (phase_ == BattlePhase::Ready) {
        renderer.fillRect({184, 250, 392, 118}, {0, 0, 0, 220});
        renderer.drawRect({184, 250, 392, 118}, {255, 215, 92, 255});
        renderer.drawText({240, 278}, "BATTLE CITY", {255, 235, 130, 255}, 3);
        renderer.drawText({236, 332}, "MOVE OR SPACE TO START", {235, 244, 255, 255}, 1);
    } else if (phase_ == BattlePhase::Paused) {
        renderer.fillRect({250, 282, 260, 74}, {0, 0, 0, 215});
        renderer.drawRect({250, 282, 260, 74}, {245, 250, 255, 255});
        renderer.drawText({302, 306}, "PAUSED", {245, 250, 255, 255}, 3);
    } else if (phase_ == BattlePhase::StageCleared) {
        renderer.fillRect({196, 272, 368, 88}, {0, 0, 0, 220});
        renderer.drawRect({196, 272, 368, 88}, {120, 255, 160, 255});
        renderer.drawText({238, 302}, "STAGE CLEAR", {150, 255, 180, 255}, 3);
    } else if (phase_ == BattlePhase::GameOver) {
        renderer.fillRect({178, 256, 404, 122}, {0, 0, 0, 225});
        renderer.drawRect({178, 256, 404, 122}, {255, 90, 90, 255});
        renderer.drawText({238, 284}, "GAME OVER", {255, 120, 120, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    }

    renderer.endFrame();
}

auto BattleCityGame::wantsToQuit() const -> bool { return quit_; }

void BattleCityGame::buildStage() {
    tiles_.fill(Tile::Empty);
    for (int x = 0; x < BoardCols; ++x) {
        setTile(x, 0, Tile::Steel); setTile(x, BoardRows - 1, Tile::Steel);
    }
    for (int y = 0; y < BoardRows; ++y) {
        setTile(0, y, Tile::Steel); setTile(BoardCols - 1, y, Tile::Steel);
    }

    for (int y = 3; y < 18; y += 3) {
        for (int x = 3; x < 23; x += 4) {
            setTile(x, y, Tile::Brick); setTile(x + 1, y, Tile::Brick);
            if ((x + y + stage_) % 2 == 0) { setTile(x, y + 1, Tile::Brick); }
        }
    }
    for (int x = 6; x < 20; ++x) {
        if (x != 12 && x != 13) { setTile(x, 10, Tile::Steel); }
    }
    for (int x = 2; x < 8; ++x) { setTile(x, 7, Tile::Water); }
    for (int x = 18; x < 24; ++x) { setTile(x, 14, Tile::Water); }
    for (int y = 5; y < 9; ++y) { setTile(12, y, Tile::Trees); setTile(13, y, Tile::Trees); }
    for (int y = 15; y < 18; ++y) { setTile(5, y, Tile::Trees); setTile(20, y, Tile::Trees); }

    // Eagle base and protective brick wall.
    setTile(12, 20, Tile::Base); setTile(13, 20, Tile::Base);
    setTile(12, 21, Tile::Base); setTile(13, 21, Tile::Base);
    setTile(11, 19, Tile::Brick); setTile(12, 19, Tile::Brick); setTile(13, 19, Tile::Brick); setTile(14, 19, Tile::Brick);
    setTile(11, 20, Tile::Brick); setTile(14, 20, Tile::Brick);
    setTile(11, 21, Tile::Brick); setTile(14, 21, Tile::Brick);
}

void BattleCityGame::spawnEnemy() {
    static constexpr int spawnCols[] = {2, 12, 22, 6, 18};
    const int col = spawnCols[spawnCursor_ % 5];
    ++spawnCursor_;
    const Vec2d pos{BoardLeft + static_cast<double>(col * TileSize) + 1.0, BoardTop + 1.0 * TileSize + 1.0};
    if (!canMoveTo(pos, false)) { return; }
    enemies_.push_back(Tank{.position = pos, .direction = Direction::Down, .speed = 72.0 + stage_ * 5.0, .player = false, .alive = true, .fireCooldown = 0.8, .aiTimer = 0.4, .hp = 1});
    --enemiesRemaining_;
}

void BattleCityGame::updatePlayer(const double dt) {
    if (!player_.alive) { return; }
    if (fireRequested_) { fire(player_, true); fireRequested_ = false; }
    if (!hasMoveInput_) { return; }

    player_.direction = requestedDirection_;
    const auto v = directionVector(player_.direction);
    const Vec2d next{player_.position.x + v.x * player_.speed * dt, player_.position.y + v.y * player_.speed * dt};
    if (canMoveTo(next, true)) { player_.position = next; }
}

void BattleCityGame::updateEnemies(const double dt) {
    for (auto& enemy : enemies_) {
        enemy.fireCooldown = std::max(0.0, enemy.fireCooldown - dt);
        enemy.aiTimer -= dt;
        if (enemy.aiTimer <= 0.0) {
            const auto selector = (static_cast<int>(enemy.position.x + enemy.position.y) + score_ + stage_ * 17) % 5;
            enemy.direction = selector == 0 ? Direction::Left : selector == 1 ? Direction::Right : selector == 2 ? Direction::Up : Direction::Down;
            enemy.aiTimer = 0.45 + static_cast<double>(selector) * 0.18;
            if (selector == 4 || enemy.fireCooldown <= 0.0) { fire(enemy, false); }
        }
        const auto v = directionVector(enemy.direction);
        const Vec2d next{enemy.position.x + v.x * enemy.speed * dt, enemy.position.y + v.y * enemy.speed * dt};
        if (canMoveTo(next, false)) { enemy.position = next; }
        else { enemy.aiTimer = 0.0; }
    }
    std::erase_if(enemies_, [](const Tank& tank) { return !tank.alive; });
}

void BattleCityGame::updateBullets(const double dt) {
    for (auto& bullet : bullets_) {
        if (!bullet.active) { continue; }
        const auto v = directionVector(bullet.direction);
        bullet.position.x += v.x * bullet.speed * dt;
        bullet.position.y += v.y * bullet.speed * dt;
        resolveBullet(bullet);
    }
    std::erase_if(bullets_, [](const Bullet& bullet) { return !bullet.active; });
}

void BattleCityGame::fire(Tank& tank, const bool fromPlayer) {
    if (tank.fireCooldown > 0.0 || !tank.alive) { return; }
    if (fromPlayer && std::ranges::any_of(bullets_, [](const Bullet& b) { return b.fromPlayer && b.active; })) { return; }
    const auto v = directionVector(tank.direction);
    bullets_.push_back(Bullet{.position = {tank.position.x + TankSize / 2.0 + v.x * 14.0, tank.position.y + TankSize / 2.0 + v.y * 14.0}, .direction = tank.direction, .fromPlayer = fromPlayer, .active = true, .speed = fromPlayer ? 360.0 : 280.0});
    tank.fireCooldown = fromPlayer ? 0.28 : 1.1;
}

void BattleCityGame::resolveBullet(Bullet& bullet) {
    const auto bulletRect = worldRect({bullet.position.x - BulletSize / 2.0, bullet.position.y - BulletSize / 2.0}, BulletSize);
    if (bulletRect.x < BoardLeft || bulletRect.y < BoardTop || bulletRect.x + bulletRect.width > BoardLeft + BoardCols * TileSize || bulletRect.y + bulletRect.height > BoardTop + BoardRows * TileSize) {
        bullet.active = false; return;
    }

    const int minCol = std::clamp(toTileCoord(static_cast<double>(bulletRect.x), BoardLeft), 0, BoardCols - 1);
    const int maxCol = std::clamp(toTileCoord(static_cast<double>(bulletRect.x + bulletRect.width - 1), BoardLeft), 0, BoardCols - 1);
    const int minRow = std::clamp(toTileCoord(static_cast<double>(bulletRect.y), BoardTop), 0, BoardRows - 1);
    const int maxRow = std::clamp(toTileCoord(static_cast<double>(bulletRect.y + bulletRect.height - 1), BoardTop), 0, BoardRows - 1);
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            const auto tile = tileAt(col, row);
            if (!tileBlocksBullet(tile)) { continue; }
            if (tile == Tile::Brick) { setTile(col, row, Tile::Empty); addExplosion({static_cast<double>(BoardLeft + col * TileSize + TileSize / 2), static_cast<double>(BoardTop + row * TileSize + TileSize / 2)}, 0.32); }
            else { addExplosion(bullet.position, 0.24); }
            if (tile == Tile::Base) { destroyBase(); }
            bullet.active = false; return;
        }
    }

    if (bullet.fromPlayer) {
        for (auto& enemy : enemies_) {
            if (enemy.alive && intersects(bulletRect, worldRect(enemy.position, TankSize))) {
                enemy.alive = false; bullet.active = false; addExplosion({enemy.position.x + TankSize / 2.0, enemy.position.y + TankSize / 2.0}, 0.55); score_ += 100; return;
            }
        }
    } else if (player_.alive && intersects(bulletRect, worldRect(player_.position, TankSize))) {
        bullet.active = false; addExplosion({player_.position.x + TankSize / 2.0, player_.position.y + TankSize / 2.0}, 0.55); loseLife(); return;
    }

    for (auto& other : bullets_) {
        if (&other != &bullet && other.active && other.fromPlayer != bullet.fromPlayer) {
            const auto otherRect = worldRect({other.position.x - BulletSize / 2.0, other.position.y - BulletSize / 2.0}, BulletSize);
            if (intersects(bulletRect, otherRect)) { addExplosion(bullet.position, 0.22); other.active = false; bullet.active = false; return; }
        }
    }
}

void BattleCityGame::loseLife() {
    --lives_;
    bullets_.clear();
    if (lives_ <= 0) {
        phase_ = BattlePhase::GameOver;
        events_.publish({"battle_city.game_over", score_});
        return;
    }
    player_.position = {BoardLeft + 9.0 * TileSize + 1.0, BoardTop + 20.0 * TileSize + 1.0};
    player_.direction = Direction::Up;
    player_.fireCooldown = 0.0;
}

void BattleCityGame::clearStage() {
    phase_ = BattlePhase::StageCleared;
    messageTimer_ = 1.3;
    score_ += 500;
    events_.publish({"battle_city.stage_cleared", score_});
}

void BattleCityGame::destroyBase() {
    addExplosion({BoardLeft + 13.0 * TileSize, BoardTop + 21.0 * TileSize}, 0.75);
    for (int row = 20; row <= 21; ++row) {
        for (int col = 12; col <= 13; ++col) { setTile(col, row, Tile::DestroyedBase); }
    }
    phase_ = BattlePhase::GameOver;
    events_.publish({"battle_city.game_over", score_});
}

auto BattleCityGame::canMoveTo(const Vec2d position, const bool playerTank) const -> bool {
    const auto rect = worldRect(position, TankSize);
    if (rect.x < BoardLeft || rect.y < BoardTop || rect.x + rect.width > BoardLeft + BoardCols * TileSize || rect.y + rect.height > BoardTop + BoardRows * TileSize) { return false; }

    const int minCol = std::clamp(toTileCoord(static_cast<double>(rect.x), BoardLeft), 0, BoardCols - 1);
    const int maxCol = std::clamp(toTileCoord(static_cast<double>(rect.x + rect.width - 1), BoardLeft), 0, BoardCols - 1);
    const int minRow = std::clamp(toTileCoord(static_cast<double>(rect.y), BoardTop), 0, BoardRows - 1);
    const int maxRow = std::clamp(toTileCoord(static_cast<double>(rect.y + rect.height - 1), BoardTop), 0, BoardRows - 1);
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            if (tileBlocksTank(tileAt(col, row))) { return false; }
        }
    }

    if (playerTank) {
        for (const auto& enemy : enemies_) {
            if (enemy.alive && intersects(rect, worldRect(enemy.position, TankSize))) { return false; }
        }
    } else if (player_.alive && intersects(rect, worldRect(player_.position, TankSize))) { return false; }
    return true;
}

auto BattleCityGame::tileAt(const int col, const int row) const -> Tile {
    if (col < 0 || col >= BoardCols || row < 0 || row >= BoardRows) { return Tile::Steel; }
    return tiles_[static_cast<std::size_t>(row * BoardCols + col)];
}

void BattleCityGame::setTile(const int col, const int row, const Tile tile) {
    if (col < 0 || col >= BoardCols || row < 0 || row >= BoardRows) { return; }
    tiles_[static_cast<std::size_t>(row * BoardCols + col)] = tile;
}

auto BattleCityGame::worldRect(const Vec2d position, const int size) const -> gamecore::Recti {
    return {static_cast<int>(std::lround(position.x)), static_cast<int>(std::lround(position.y)), size, size};
}

auto BattleCityGame::directionVector(const Direction direction) -> Vec2d {
    switch (direction) {
        case Direction::Up: return {0.0, -1.0};
        case Direction::Right: return {1.0, 0.0};
        case Direction::Down: return {0.0, 1.0};
        case Direction::Left: return {-1.0, 0.0};
    }
    return {0.0, -1.0};
}

auto BattleCityGame::intersects(const gamecore::Recti a, const gamecore::Recti b) -> bool {
    return a.x < b.x + b.width && a.x + a.width > b.x && a.y < b.y + b.height && a.y + a.height > b.y;
}

auto BattleCityGame::toTileCoord(const double world, const int origin) -> int {
    return static_cast<int>(std::floor((world - static_cast<double>(origin)) / static_cast<double>(TileSize)));
}

void BattleCityGame::addExplosion(const Vec2d position, const double duration) {
    explosions_.push_back(Explosion{.position = position, .age = 0.0, .duration = duration});
}

void BattleCityGame::drawTile(gamecore::IRenderer2D& renderer, const int col, const int row, const Tile tile) const {
    const gamecore::Recti r{BoardLeft + col * TileSize, BoardTop + row * TileSize, TileSize, TileSize};
    switch (tile) {
        case Tile::Brick:
            renderer.fillRect(r, {128, 60, 35, 255});
            renderer.fillRect({r.x + 1, r.y + 1, r.width - 2, 6}, {184, 96, 52, 255});
            renderer.fillRect({r.x + 1, r.y + 9, r.width - 2, 6}, {168, 82, 45, 255});
            renderer.fillRect({r.x + 1, r.y + 17, r.width - 2, 6}, {150, 70, 40, 255});
            renderer.drawLine({r.x, r.y + 8}, {r.x + r.width, r.y + 8}, {70, 35, 25, 255});
            renderer.drawLine({r.x, r.y + 16}, {r.x + r.width, r.y + 16}, {70, 35, 25, 255});
            renderer.drawLine({r.x + 12, r.y}, {r.x + 12, r.y + 8}, {70, 35, 25, 255});
            renderer.drawLine({r.x + 6, r.y + 8}, {r.x + 6, r.y + 16}, {70, 35, 25, 255});
            renderer.drawLine({r.x + 18, r.y + 16}, {r.x + 18, r.y + 24}, {70, 35, 25, 255});
            renderer.drawLine({r.x + 2, r.y + 2}, {r.x + 9, r.y + 2}, {220, 132, 82, 150});
            break;
        case Tile::Steel:
            renderer.fillRect(r, {86, 94, 108, 255});
            renderer.fillRect({r.x + 2, r.y + 2, 9, 9}, {142, 153, 170, 255});
            renderer.fillRect({r.x + 13, r.y + 2, 9, 9}, {112, 122, 140, 255});
            renderer.fillRect({r.x + 2, r.y + 13, 9, 9}, {112, 122, 140, 255});
            renderer.fillRect({r.x + 13, r.y + 13, 9, 9}, {142, 153, 170, 255});
            renderer.drawRect({r.x + 2, r.y + 2, r.width - 4, r.height - 4}, {210, 220, 232, 255});
            renderer.drawLine({r.x, r.y + r.height - 1}, {r.x + r.width, r.y}, {48, 54, 66, 255});
            break;
        case Tile::Water: {
            renderer.fillRect(r, {13, 50, 120, 255});
            const auto wave = static_cast<int>(std::lround(std::sin(animationTime_ * 5.0 + col + row) * 2.0));
            renderer.drawLine({r.x + 2, r.y + 6 + wave}, {r.x + 11, r.y + 4 + wave}, {66, 166, 245, 230});
            renderer.drawLine({r.x + 12, r.y + 4 - wave}, {r.x + 22, r.y + 7 - wave}, {66, 166, 245, 210});
            renderer.drawLine({r.x + 3, r.y + 16 - wave}, {r.x + 20, r.y + 14 - wave}, {110, 205, 255, 170});
            renderer.fillRect({r.x, r.y + 21, r.width, 3}, {7, 30, 78, 190});
            break;
        }
        case Tile::Trees: {
            renderer.fillRect(r, {18, 65, 30, 170});
            const auto sway = static_cast<int>(std::lround(std::sin(animationTime_ * 3.0 + col * 0.7) * 1.5));
            renderer.fillCircle({r.x + 6 + sway, r.y + 8}, 7, {42, 138, 54, 235});
            renderer.fillCircle({r.x + 15 - sway, r.y + 9}, 8, {30, 116, 44, 235});
            renderer.fillCircle({r.x + 11, r.y + 16}, 7, {22, 96, 36, 235});
            renderer.drawLine({r.x + 11, r.y + 12}, {r.x + 11, r.y + 22}, {74, 54, 30, 190});
            break;
        }
        case Tile::Base:
            drawBase(renderer, col, row, false);
            break;
        case Tile::DestroyedBase:
            drawBase(renderer, col, row, true);
            break;
        case Tile::Empty: break;
    }
}

void BattleCityGame::drawTank(gamecore::IRenderer2D& renderer, const Tank& tank) const {
    const auto r = worldRect(tank.position, TankSize);
    const auto c = baseColor(tank.player);
    const auto dark = tank.player ? gamecore::Color{88, 74, 38, 255} : gamecore::Color{34, 76, 44, 255};
    const auto darker = tank.player ? gamecore::Color{42, 35, 24, 255} : gamecore::Color{14, 34, 24, 255};
    const auto light = tank.player ? gamecore::Color{255, 236, 132, 255} : gamecore::Color{132, 238, 142, 255};
    const auto highlight = tank.player ? gamecore::Color{255, 250, 182, 235} : gamecore::Color{186, 255, 188, 220};

    // Tracks.
    renderer.fillRect({r.x + 1, r.y + 2, 5, r.height - 4}, darker);
    renderer.fillRect({r.x + r.width - 6, r.y + 2, 5, r.height - 4}, darker);
    const auto treadPhase = static_cast<int>(std::floor((tank.position.x + tank.position.y + animationTime_ * (tank.player ? 80.0 : 55.0)) / 5.0)) % 2;
    for (int i = 0; i < 4; ++i) {
        const auto y = r.y + 3 + i * 5 + treadPhase * 2;
        renderer.drawLine({r.x + 1, y}, {r.x + 5, y}, dark);
        renderer.drawLine({r.x + r.width - 6, y}, {r.x + r.width - 2, y}, dark);
    }

    // Armored hull.
    renderer.fillRect({r.x + 5, r.y + 4, r.width - 10, r.height - 8}, c);
    renderer.fillTriangle({r.x + 5, r.y + 4}, {r.x + r.width / 2, r.y + 1}, {r.x + r.width - 5, r.y + 4}, highlight);
    renderer.fillRect({r.x + 7, r.y + 15, r.width - 14, 3}, dark);
    renderer.drawRect({r.x + 5, r.y + 4, r.width - 10, r.height - 8}, darker);

    const auto cx = r.x + r.width / 2;
    const auto cy = r.y + r.height / 2;
    renderer.fillCircle({cx, cy}, 7, light);
    renderer.drawCircle({cx, cy}, 7, darker);
    renderer.fillCircle({cx - 2, cy - 2}, 2, highlight);

    switch (tank.direction) {
        case Direction::Up:
            renderer.fillRect({cx - 2, r.y - 6, 4, 14}, light);
            renderer.drawRect({cx - 2, r.y - 6, 4, 14}, darker);
            break;
        case Direction::Down:
            renderer.fillRect({cx - 2, r.y + r.height - 8, 4, 14}, light);
            renderer.drawRect({cx - 2, r.y + r.height - 8, 4, 14}, darker);
            break;
        case Direction::Left:
            renderer.fillRect({r.x - 6, cy - 2, 14, 4}, light);
            renderer.drawRect({r.x - 6, cy - 2, 14, 4}, darker);
            break;
        case Direction::Right:
            renderer.fillRect({r.x + r.width - 8, cy - 2, 14, 4}, light);
            renderer.drawRect({r.x + r.width - 8, cy - 2, 14, 4}, darker);
            break;
    }

    // Tiny engine glow when moving.
    const auto v = directionVector(tank.direction);
    const gamecore::Vec2i back{cx - static_cast<int>(v.x * 10.0), cy - static_cast<int>(v.y * 10.0)};
    renderer.fillCircle(back, 2, tank.player ? gamecore::Color{255, 135, 55, 190} : gamecore::Color{130, 255, 110, 150});
}

void BattleCityGame::drawExplosion(gamecore::IRenderer2D& renderer, const Explosion& explosion) const {
    const auto t = std::clamp(explosion.age / explosion.duration, 0.0, 1.0);
    const auto cx = static_cast<int>(std::lround(explosion.position.x));
    const auto cy = static_cast<int>(std::lround(explosion.position.y));
    const auto outer = 4 + static_cast<int>(std::lround(t * 18.0));
    const auto inner = std::max(2, outer / 2);
    renderer.fillCircle({cx, cy}, outer, {255, 100, 42, static_cast<std::uint8_t>(std::lround(210.0 * (1.0 - t)))});
    renderer.fillCircle({cx, cy}, inner, {255, 230, 105, static_cast<std::uint8_t>(std::lround(240.0 * (1.0 - t)))});
    renderer.drawLine({cx - outer, cy}, {cx + outer, cy}, {255, 230, 105, static_cast<std::uint8_t>(std::lround(220.0 * (1.0 - t)))});
    renderer.drawLine({cx, cy - outer}, {cx, cy + outer}, {255, 230, 105, static_cast<std::uint8_t>(std::lround(220.0 * (1.0 - t)))});
}

void BattleCityGame::drawBase(gamecore::IRenderer2D& renderer, const int col, const int row, const bool destroyed) const {
    const gamecore::Recti r{BoardLeft + col * TileSize, BoardTop + row * TileSize, TileSize, TileSize};
    renderer.fillRect(r, destroyed ? gamecore::Color{70, 54, 48, 255} : gamecore::Color{180, 150, 88, 255});
    if (destroyed) {
        renderer.drawLine({r.x + 4, r.y + 4}, {r.x + 20, r.y + 20}, {35, 28, 24, 255});
        renderer.drawLine({r.x + 20, r.y + 4}, {r.x + 4, r.y + 20}, {35, 28, 24, 255});
    } else {
        renderer.fillTriangle({r.x + 12, r.y + 4}, {r.x + 5, r.y + 18}, {r.x + 19, r.y + 18}, {255, 236, 166, 255});
        renderer.fillRect({r.x + 9, r.y + 12, 6, 8}, {82, 56, 28, 255});
    }
}

} // namespace games::battle_city
