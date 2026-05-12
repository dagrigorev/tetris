#include "games/arkanoid/ArkanoidGame.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace games::arkanoid {
namespace {

[[nodiscard]] auto intersects(const FloatRect& a, const FloatRect& b) -> bool {
    return a.left() < b.right() && a.right() > b.left() && a.top() < b.bottom() && a.bottom() > b.top();
}

[[nodiscard]] auto ballBounds(const Ball& ball) -> FloatRect {
    return {ball.x - ball.radius, ball.y - ball.radius, ball.radius * 2.0, ball.radius * 2.0};
}

[[nodiscard]] auto clampAbsSpeed(double value, const double minimum) -> double {
    if (std::abs(value) >= minimum) {
        return value;
    }
    return value < 0.0 ? -minimum : minimum;
}

} // namespace

ArkanoidGame::ArkanoidGame(gamecore::EventBus& events) : events_(events) {
    restart();
}

auto ArkanoidGame::name() const -> std::string_view {
    return "Arkanoid";
}

void ArkanoidGame::restart() {
    score_ = 0;
    lives_ = 3;
    phase_ = ArkanoidPhase::Ready;
    quit_ = false;
    moveDirection_ = 0;
    paddle_ = Paddle{};
    buildLevel();
    resetRound();
}

void ArkanoidGame::handleInput(const gamecore::InputFrame& input) {
    using enum gamecore::InputCommand;

    if (input.isPressed(Quit)) {
        quit_ = true;
        return;
    }

    if (input.isPressed(Restart)) {
        restart();
        return;
    }

    if (input.isPressed(Pause) && (phase_ == ArkanoidPhase::Playing || phase_ == ArkanoidPhase::Paused)) {
        phase_ = phase_ == ArkanoidPhase::Paused ? ArkanoidPhase::Playing : ArkanoidPhase::Paused;
        return;
    }

    moveDirection_ = 0;
    if (input.isDown(MoveLeft)) {
        moveDirection_ -= 1;
    }
    if (input.isDown(MoveRight)) {
        moveDirection_ += 1;
    }

    if ((input.isPressed(Select) || input.isPressed(HardDrop)) && phase_ == ArkanoidPhase::Ready) {
        launchBall();
    }
}

void ArkanoidGame::update(const gamecore::Seconds deltaTime) {
    const auto dt = std::clamp(deltaTime.count(), 0.0, 0.035);

    if (phase_ == ArkanoidPhase::Ready || phase_ == ArkanoidPhase::Playing) {
        updatePaddle(dt);
    }

    if (phase_ == ArkanoidPhase::Ready) {
        ball_.x = paddle_.rect.x + paddle_.rect.width / 2.0;
        ball_.y = paddle_.rect.y - ball_.radius - 3.0;
        return;
    }

    if (phase_ != ArkanoidPhase::Playing) {
        return;
    }

    updateBall(dt);

    if (aliveBrickCount() == 0) {
        phase_ = ArkanoidPhase::Won;
        events_.publish({"arkanoid.level_completed", score_});
    }
}

void ArkanoidGame::render(gamecore::IRenderer2D& renderer) const {
    renderer.beginFrame({4, 6, 16, 255});

    renderer.fillRect({24, 28, ScreenWidth - 48, ScreenHeight - 56}, {9, 12, 28, 255});
    renderer.drawRect({WallLeft - 4, WallTop - 4, WallRight - WallLeft + 8, ScreenHeight - WallTop - 28}, {80, 100, 150, 255});
    renderer.drawLine({WallLeft, WallTop}, {WallRight, WallTop}, {150, 190, 255, 255});
    renderer.drawLine({WallLeft, WallTop}, {WallLeft, ScreenHeight - 26}, {80, 110, 175, 255});
    renderer.drawLine({WallRight, WallTop}, {WallRight, ScreenHeight - 26}, {80, 110, 175, 255});

    for (const auto& brick : bricks_) {
        if (!brick.alive()) {
            continue;
        }
        renderer.fillRect(toRect(brick.rect), brick.color);
        renderer.drawRect(toRect(brick.rect), {245, 250, 255, 155});
    }

    renderer.fillRect(toRect(paddle_.rect), {92, 180, 255, 255});
    renderer.drawRect(toRect(paddle_.rect), {235, 248, 255, 255});

    renderer.fillRect(ballRect(ball_), {245, 250, 255, 255});
    renderer.drawRect(ballRect(ball_), {135, 190, 255, 255});

    renderer.drawText({48, 10}, "ARKANOID", {235, 244, 255, 255}, 2);
    renderer.drawText({236, 10}, std::string{"SCORE "} + std::to_string(score_), {190, 210, 240, 255}, 2);
    renderer.drawText({518, 10}, std::string{"LIVES "} + std::to_string(lives_), {190, 210, 240, 255}, 2);

    renderer.drawText({48, 604}, "A D OR ARROWS MOVE   SPACE START   P PAUSE   R RESTART   ESC MENU", {132, 150, 185, 255}, 1);

    if (phase_ == ArkanoidPhase::Ready) {
        renderer.fillRect({206, 278, 348, 78}, {0, 0, 0, 205});
        renderer.drawRect({206, 278, 348, 78}, {120, 180, 255, 255});
        renderer.drawText({246, 302}, "PRESS SPACE", {245, 250, 255, 255}, 3);
        renderer.drawText({286, 338}, "TO LAUNCH", {160, 200, 255, 255}, 1);
    }

    if (phase_ == ArkanoidPhase::Paused) {
        renderer.fillRect({250, 282, 260, 74}, {0, 0, 0, 215});
        renderer.drawRect({250, 282, 260, 74}, {245, 250, 255, 255});
        renderer.drawText({302, 306}, "PAUSED", {245, 250, 255, 255}, 3);
    }

    if (phase_ == ArkanoidPhase::Won) {
        renderer.fillRect({196, 256, 368, 122}, {0, 0, 0, 225});
        renderer.drawRect({196, 256, 368, 122}, {120, 255, 180, 255});
        renderer.drawText({264, 284}, "YOU WIN", {150, 255, 190, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    }

    if (phase_ == ArkanoidPhase::GameOver) {
        renderer.fillRect({178, 256, 404, 122}, {0, 0, 0, 225});
        renderer.drawRect({178, 256, 404, 122}, {255, 90, 90, 255});
        renderer.drawText({238, 284}, "GAME OVER", {255, 120, 120, 255}, 3);
        renderer.drawText({252, 338}, "R TO RESTART", {245, 250, 255, 255}, 2);
    }

    renderer.endFrame();
}

auto ArkanoidGame::wantsToQuit() const -> bool {
    return quit_;
}

void ArkanoidGame::resetRound() {
    paddle_.rect.x = 330.0;
    paddle_.rect.y = 578.0;
    ball_ = Ball{};
    phase_ = ArkanoidPhase::Ready;
}

void ArkanoidGame::buildLevel() {
    bricks_.clear();

    static constexpr int columns = 10;
    static constexpr int rows = 6;
    static constexpr int brickWidth = 62;
    static constexpr int brickHeight = 22;
    static constexpr int gap = 6;
    static constexpr int startX = 58;
    static constexpr int startY = 82;

    const gamecore::Color rowColors[rows] = {
        {245, 92, 92, 255},
        {245, 145, 70, 255},
        {240, 210, 85, 255},
        {90, 210, 135, 255},
        {80, 175, 240, 255},
        {165, 120, 245, 255}
    };

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            const auto hp = row < 2 ? 2 : 1;
            bricks_.push_back(Brick{
                .rect = FloatRect{static_cast<double>(startX + col * (brickWidth + gap)),
                                  static_cast<double>(startY + row * (brickHeight + gap)),
                                  static_cast<double>(brickWidth),
                                  static_cast<double>(brickHeight)},
                .hitPoints = hp,
                .score = hp == 2 ? 90 : 50,
                .color = rowColors[row]
            });
        }
    }
}

void ArkanoidGame::launchBall() {
    phase_ = ArkanoidPhase::Playing;
    ball_.velocityX = 210.0;
    ball_.velocityY = -330.0;
}

void ArkanoidGame::updatePaddle(const double dt) {
    paddle_.rect.x += static_cast<double>(moveDirection_) * paddle_.speed * dt;
    paddle_.rect.x = std::clamp(paddle_.rect.x, static_cast<double>(WallLeft), static_cast<double>(WallRight) - paddle_.rect.width);
}

void ArkanoidGame::updateBall(const double dt) {
    ball_.x += ball_.velocityX * dt;
    ball_.y += ball_.velocityY * dt;

    if (ball_.x - ball_.radius <= WallLeft) {
        ball_.x = WallLeft + ball_.radius;
        ball_.velocityX = std::abs(ball_.velocityX);
    }
    if (ball_.x + ball_.radius >= WallRight) {
        ball_.x = WallRight - ball_.radius;
        ball_.velocityX = -std::abs(ball_.velocityX);
    }
    if (ball_.y - ball_.radius <= WallTop) {
        ball_.y = WallTop + ball_.radius;
        ball_.velocityY = std::abs(ball_.velocityY);
    }
    if (ball_.y - ball_.radius > ScreenHeight) {
        loseLife();
        return;
    }

    resolvePaddleCollision();
    resolveBrickCollisions();
}

void ArkanoidGame::resolvePaddleCollision() {
    if (ball_.velocityY <= 0.0 || !intersects(ballBounds(ball_), paddle_.rect)) {
        return;
    }

    const auto paddleCenter = paddle_.rect.x + paddle_.rect.width / 2.0;
    const auto offset = std::clamp((ball_.x - paddleCenter) / (paddle_.rect.width / 2.0), -1.0, 1.0);
    const auto speed = std::sqrt(ball_.velocityX * ball_.velocityX + ball_.velocityY * ball_.velocityY) + 8.0;

    ball_.velocityX = clampAbsSpeed(offset * speed * 0.78, 95.0);
    ball_.velocityY = -std::sqrt(std::max(160.0 * 160.0, speed * speed - ball_.velocityX * ball_.velocityX));
    ball_.y = paddle_.rect.y - ball_.radius - 1.0;
}

void ArkanoidGame::resolveBrickCollisions() {
    const auto bounds = ballBounds(ball_);
    for (auto& brick : bricks_) {
        if (!brick.alive() || !intersects(bounds, brick.rect)) {
            continue;
        }

        --brick.hitPoints;
        if (!brick.alive()) {
            score_ += brick.score;
        } else {
            score_ += 20;
            brick.color = {210, 215, 225, 255};
        }

        const auto ballCenterX = ball_.x;
        const auto ballCenterY = ball_.y;
        const auto brickCenterX = brick.rect.x + brick.rect.width / 2.0;
        const auto brickCenterY = brick.rect.y + brick.rect.height / 2.0;
        const auto overlapX = (brick.rect.width / 2.0 + ball_.radius) - std::abs(ballCenterX - brickCenterX);
        const auto overlapY = (brick.rect.height / 2.0 + ball_.radius) - std::abs(ballCenterY - brickCenterY);

        if (overlapX < overlapY) {
            ball_.velocityX = -ball_.velocityX;
        } else {
            ball_.velocityY = -ball_.velocityY;
        }
        return;
    }
}

void ArkanoidGame::loseLife() {
    --lives_;
    if (lives_ <= 0) {
        phase_ = ArkanoidPhase::GameOver;
        events_.publish({"arkanoid.game_over", score_});
        return;
    }
    resetRound();
}

auto ArkanoidGame::aliveBrickCount() const -> int {
    return static_cast<int>(std::ranges::count_if(bricks_, [](const Brick& brick) { return brick.alive(); }));
}

auto ArkanoidGame::toRect(const FloatRect rect) -> gamecore::Recti {
    return {
        static_cast<int>(std::lround(rect.x)),
        static_cast<int>(std::lround(rect.y)),
        static_cast<int>(std::lround(rect.width)),
        static_cast<int>(std::lround(rect.height))
    };
}

auto ArkanoidGame::ballRect(const Ball& ball) -> gamecore::Recti {
    return {
        static_cast<int>(std::lround(ball.x - ball.radius)),
        static_cast<int>(std::lround(ball.y - ball.radius)),
        static_cast<int>(std::lround(ball.radius * 2.0)),
        static_cast<int>(std::lround(ball.radius * 2.0))
    };
}

} // namespace games::arkanoid
