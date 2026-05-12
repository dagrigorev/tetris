#include "games/galaga/GalagaGame.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace games::galaga {
namespace {

[[nodiscard]] auto intersects(const FloatRect& a, const FloatRect& b) -> bool {
    return a.left() < b.right() && a.right() > b.left() && a.top() < b.bottom() && a.bottom() > b.top();
}

[[nodiscard]] auto colorForRow(const int row) -> gamecore::Color {
    switch (row) {
        case 0: return {245, 90, 125, 255};
        case 1: return {245, 160, 70, 255};
        case 2: return {95, 205, 250, 255};
        default: return {145, 235, 125, 255};
    }
}

void drawEnemyShip(gamecore::IRenderer2D& renderer, const gamecore::Recti rect, const int row) {
    const auto body = colorForRow(row);
    const gamecore::Color highlight{245, 250, 255, 210};
    const gamecore::Color cockpit{12, 18, 42, 255};
    const gamecore::Color glow{115, 235, 255, 210};

    const auto cx = rect.x + rect.width / 2;
    const auto top = rect.y;
    const auto bottom = rect.y + rect.height;

    renderer.fillTriangle({cx, top + 2}, {rect.x + 5, bottom - 6}, {rect.x + rect.width - 5, bottom - 6}, body);
    renderer.fillTriangle({rect.x + 3, rect.y + 12}, {rect.x + 13, bottom - 2}, {rect.x + 20, rect.y + 18}, body);
    renderer.fillTriangle({rect.x + rect.width - 3, rect.y + 12}, {rect.x + rect.width - 13, bottom - 2}, {rect.x + rect.width - 20, rect.y + 18}, body);
    renderer.fillCircle({cx, rect.y + 13}, 7, body);
    renderer.fillCircle({cx, rect.y + 12}, 4, cockpit);
    renderer.fillRect({cx - 8, bottom - 7, 16, 3}, highlight);
    renderer.fillCircle({rect.x + 7, bottom - 5}, 3, glow);
    renderer.fillCircle({rect.x + rect.width - 7, bottom - 5}, 3, glow);
}

void drawPlayerShip(gamecore::IRenderer2D& renderer, const gamecore::Recti ship) {
    const auto cx = ship.x + ship.width / 2;
    const auto top = ship.y;
    const auto bottom = ship.y + ship.height;

    renderer.fillTriangle({cx, top}, {ship.x + 7, bottom - 4}, {ship.x + ship.width - 7, bottom - 4}, {90, 175, 255, 255});
    renderer.fillTriangle({ship.x, ship.y + 18}, {ship.x + 14, ship.y + 12}, {ship.x + 10, bottom}, {125, 220, 250, 255});
    renderer.fillTriangle({ship.x + ship.width, ship.y + 18}, {ship.x + ship.width - 14, ship.y + 12}, {ship.x + ship.width - 10, bottom}, {125, 220, 250, 255});
    renderer.fillCircle({cx, ship.y + 13}, 7, {190, 230, 255, 255});
    renderer.fillCircle({cx, ship.y + 13}, 4, {18, 34, 74, 255});
    renderer.fillRect({cx - 3, bottom - 6, 6, 6}, {255, 170, 65, 255});
    renderer.fillRect({cx - 1, bottom - 2, 2, 6}, {255, 225, 120, 220});
    renderer.drawLine({cx, top}, {ship.x + 6, bottom - 3}, {235, 248, 255, 170});
    renderer.drawLine({cx, top}, {ship.x + ship.width - 6, bottom - 3}, {235, 248, 255, 170});
}

} // namespace

GalagaGame::GalagaGame(gamecore::EventBus& events) : events_(events) {
    restart();
}

auto GalagaGame::name() const -> std::string_view {
    return "Galaga";
}

void GalagaGame::restart() {
    score_ = 0;
    lives_ = 3;
    wave_ = 1;
    moveDirection_ = 0;
    enemyDirection_ = 1.0;
    enemyFireTimer_ = 0.9;
    waveMessageTimer_ = 0.0;
    quit_ = false;
    player_ = PlayerShip{};
    shots_.clear();
    phase_ = GalagaPhase::Ready;
    buildWave();
}

void GalagaGame::handleInput(const gamecore::InputFrame& input) {
    using enum gamecore::InputCommand;

    if (input.isPressed(Quit)) {
        quit_ = true;
        return;
    }

    if (input.isPressed(Restart)) {
        restart();
        return;
    }

    if (input.isPressed(Pause) && (phase_ == GalagaPhase::Playing || phase_ == GalagaPhase::Paused)) {
        phase_ = phase_ == GalagaPhase::Paused ? GalagaPhase::Playing : GalagaPhase::Paused;
        return;
    }

    moveDirection_ = 0;
    if (input.isDown(MoveLeft)) {
        --moveDirection_;
    }
    if (input.isDown(MoveRight)) {
        ++moveDirection_;
    }

    if ((input.isPressed(Select) || input.isPressed(HardDrop)) && phase_ == GalagaPhase::Ready) {
        phase_ = GalagaPhase::Playing;
        return;
    }

    if ((input.isPressed(Select) || input.isPressed(HardDrop)) && phase_ == GalagaPhase::Playing) {
        firePlayerShot();
    }
}

void GalagaGame::update(const gamecore::Seconds deltaTime) {
    const auto dt = std::clamp(deltaTime.count(), 0.0, 0.035);

    if (phase_ == GalagaPhase::Ready || phase_ == GalagaPhase::Playing) {
        updatePlayer(dt);
    }

    if (phase_ == GalagaPhase::WaveCleared) {
        waveMessageTimer_ -= dt;
        if (waveMessageTimer_ <= 0.0) {
            ++wave_;
            buildWave();
            phase_ = GalagaPhase::Playing;
        }
        return;
    }

    if (phase_ != GalagaPhase::Playing) {
        return;
    }

    updateEnemies(dt);
    updateShots(dt);
    enemyFireTimer_ -= dt;
    if (enemyFireTimer_ <= 0.0) {
        fireEnemyShot();
        enemyFireTimer_ = std::max(0.28, 1.0 - static_cast<double>(wave_) * 0.08);
    }

    resolveCollisions();

    if (aliveEnemyCount() == 0) {
        score_ += 500 + wave_ * 50;
        phase_ = GalagaPhase::WaveCleared;
        waveMessageTimer_ = 1.2;
        events_.publish({"galaga.wave_cleared", score_});
    }
}

void GalagaGame::render(gamecore::IRenderer2D& renderer) const {
    renderer.beginFrame({3, 4, 13, 255});

    renderer.fillRect({24, 28, ScreenWidth - 48, ScreenHeight - 56}, {7, 10, 28, 255});
    renderer.drawRect({PlayLeft - 4, PlayTop - 4, PlayRight - PlayLeft + 8, PlayBottom - PlayTop + 8}, {70, 95, 150, 255});

    for (int i = 0; i < 70; ++i) {
        const auto x = 42 + (i * 97) % 676;
        const auto y = 58 + (i * 53) % 520;
        const auto intensity = static_cast<std::uint8_t>(110 + (i * 17) % 130);
        renderer.fillRect({x, y, 1 + (i % 2), 1 + (i % 2)}, {intensity, intensity, 255, 180});
    }

    for (const auto& enemy : enemies_) {
        if (!enemy.alive) {
            continue;
        }
        const auto rect = toRect(enemy.rect);
        drawEnemyShip(renderer, rect, enemy.row);
    }

    for (const auto& shot : shots_) {
        if (!shot.active) {
            continue;
        }
        renderer.fillRect(toRect(shot.rect), shot.fromPlayer ? gamecore::Color{140, 230, 255, 255} : gamecore::Color{255, 115, 115, 255});
    }

    const auto ship = toRect(player_.rect);
    drawPlayerShip(renderer, ship);

    renderer.drawText({48, 10}, "GALAGA", {235, 244, 255, 255}, 2);
    renderer.drawText({210, 10}, std::string{"SCORE "} + std::to_string(score_), {190, 210, 240, 255}, 2);
    renderer.drawText({426, 10}, std::string{"WAVE "} + std::to_string(wave_), {190, 210, 240, 255}, 2);
    renderer.drawText({588, 10}, std::string{"LIVES "} + std::to_string(lives_), {190, 210, 240, 255}, 2);
    renderer.drawText({48, 616}, "A D OR ARROWS MOVE   SPACE FIRE/START   P PAUSE   R RESTART   ESC MENU", {132, 150, 185, 255}, 1);

    if (phase_ == GalagaPhase::Ready) {
        renderer.fillRect({190, 270, 380, 94}, {0, 0, 0, 215});
        renderer.drawRect({190, 270, 380, 94}, {125, 190, 255, 255});
        renderer.drawText({262, 294}, "GALAGA", {245, 250, 255, 255}, 4);
        renderer.drawText({250, 342}, "SPACE TO START", {160, 205, 255, 255}, 1);
    } else if (phase_ == GalagaPhase::Paused) {
        renderer.fillRect({250, 282, 260, 74}, {0, 0, 0, 215});
        renderer.drawRect({250, 282, 260, 74}, {245, 250, 255, 255});
        renderer.drawText({302, 306}, "PAUSED", {245, 250, 255, 255}, 3);
    } else if (phase_ == GalagaPhase::WaveCleared) {
        renderer.fillRect({206, 274, 348, 84}, {0, 0, 0, 215});
        renderer.drawRect({206, 274, 348, 84}, {120, 255, 180, 255});
        renderer.drawText({250, 302}, "WAVE CLEAR", {150, 255, 190, 255}, 3);
    } else if (phase_ == GalagaPhase::GameOver) {
        renderer.fillRect({178, 256, 404, 122}, {0, 0, 0, 225});
        renderer.drawRect({178, 256, 404, 122}, {255, 90, 90, 255});
        renderer.drawText({238, 284}, "GAME OVER", {255, 120, 120, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    }

    renderer.endFrame();
}

auto GalagaGame::wantsToQuit() const -> bool {
    return quit_;
}

void GalagaGame::buildWave() {
    enemies_.clear();
    shots_.clear();
    enemyDirection_ = 1.0;

    static constexpr int columns = 10;
    static constexpr int rows = 4;
    static constexpr int width = 42;
    static constexpr int height = 28;
    static constexpr int gapX = 20;
    static constexpr int gapY = 18;
    static constexpr int startX = 78;
    static constexpr int startY = 86;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            enemies_.push_back(Enemy{
                .rect = FloatRect{static_cast<double>(startX + col * (width + gapX)),
                                  static_cast<double>(startY + row * (height + gapY)),
                                  static_cast<double>(width),
                                  static_cast<double>(height)},
                .row = row,
                .score = 160 - row * 20,
                .alive = true
            });
        }
    }
}

void GalagaGame::updatePlayer(const double dt) {
    player_.rect.x += static_cast<double>(moveDirection_) * player_.speed * dt;
    player_.rect.x = std::clamp(player_.rect.x, static_cast<double>(PlayLeft), static_cast<double>(PlayRight) - player_.rect.width);
}

void GalagaGame::updateEnemies(const double dt) {
    auto hitEdge = false;
    const auto speed = 44.0 + static_cast<double>(wave_) * 8.0 + static_cast<double>(40 - aliveEnemyCount()) * 1.3;
    for (const auto& enemy : enemies_) {
        if (!enemy.alive) {
            continue;
        }
        const auto nextX = enemy.rect.x + enemyDirection_ * speed * dt;
        if (nextX <= PlayLeft || nextX + enemy.rect.width >= PlayRight) {
            hitEdge = true;
            break;
        }
    }

    if (hitEdge) {
        enemyDirection_ = -enemyDirection_;
        for (auto& enemy : enemies_) {
            enemy.rect.y += 16.0;
            if (enemy.alive && enemy.rect.bottom() >= player_.rect.top()) {
                loseLife();
                return;
            }
        }
    }

    for (auto& enemy : enemies_) {
        enemy.rect.x += enemyDirection_ * speed * dt;
    }
}

void GalagaGame::updateShots(const double dt) {
    for (auto& shot : shots_) {
        shot.rect.y += shot.velocityY * dt;
        if (shot.rect.bottom() < PlayTop || shot.rect.top() > PlayBottom) {
            shot.active = false;
        }
    }

    std::erase_if(shots_, [](const Shot& shot) { return !shot.active; });
}

void GalagaGame::firePlayerShot() {
    const auto hasPlayerShot = std::ranges::any_of(shots_, [](const Shot& shot) { return shot.fromPlayer && shot.active; });
    if (hasPlayerShot) {
        return;
    }

    shots_.push_back(Shot{
        .rect = FloatRect{player_.rect.x + player_.rect.width / 2.0 - 2.0, player_.rect.y - 12.0, 4.0, 14.0},
        .velocityY = -560.0,
        .fromPlayer = true,
        .active = true
    });
}

void GalagaGame::fireEnemyShot() {
    std::vector<const Enemy*> aliveEnemies;
    aliveEnemies.reserve(enemies_.size());
    for (const auto& enemy : enemies_) {
        if (enemy.alive) {
            aliveEnemies.push_back(&enemy);
        }
    }
    if (aliveEnemies.empty()) {
        return;
    }

    const auto index = static_cast<std::size_t>((score_ / 70 + wave_ * 5 + lives_ * 3) % static_cast<int>(aliveEnemies.size()));
    const auto& enemy = *aliveEnemies[index];
    shots_.push_back(Shot{
        .rect = FloatRect{enemy.rect.x + enemy.rect.width / 2.0 - 2.0, enemy.rect.bottom() + 3.0, 4.0, 13.0},
        .velocityY = 230.0 + static_cast<double>(wave_) * 18.0,
        .fromPlayer = false,
        .active = true
    });
}

void GalagaGame::resolveCollisions() {
    for (auto& shot : shots_) {
        if (!shot.active) {
            continue;
        }

        if (shot.fromPlayer) {
            for (auto& enemy : enemies_) {
                if (!enemy.alive || !intersects(shot.rect, enemy.rect)) {
                    continue;
                }
                enemy.alive = false;
                shot.active = false;
                score_ += enemy.score;
                break;
            }
        } else if (intersects(shot.rect, player_.rect)) {
            shot.active = false;
            loseLife();
            return;
        }
    }

    std::erase_if(shots_, [](const Shot& shot) { return !shot.active; });
}

void GalagaGame::loseLife() {
    --lives_;
    shots_.clear();
    player_.rect.x = 360.0;
    if (lives_ <= 0) {
        phase_ = GalagaPhase::GameOver;
        events_.publish({"galaga.game_over", score_});
    } else {
        phase_ = GalagaPhase::Ready;
    }
}

auto GalagaGame::aliveEnemyCount() const -> int {
    return static_cast<int>(std::ranges::count_if(enemies_, [](const Enemy& enemy) { return enemy.alive; }));
}

auto GalagaGame::toRect(const FloatRect rect) -> gamecore::Recti {
    return {
        static_cast<int>(std::lround(rect.x)),
        static_cast<int>(std::lround(rect.y)),
        static_cast<int>(std::lround(rect.width)),
        static_cast<int>(std::lround(rect.height))
    };
}

} // namespace games::galaga
