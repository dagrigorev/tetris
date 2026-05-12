#pragma once

#include <string_view>
#include <vector>

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"
#include "gamecore/Geometry.hpp"

namespace games::arkanoid {

enum class ArkanoidPhase {
    Ready,
    Playing,
    Paused,
    Won,
    GameOver
};

struct FloatRect final {
    double x{};
    double y{};
    double width{};
    double height{};

    [[nodiscard]] auto left() const -> double { return x; }
    [[nodiscard]] auto right() const -> double { return x + width; }
    [[nodiscard]] auto top() const -> double { return y; }
    [[nodiscard]] auto bottom() const -> double { return y + height; }
};

struct Paddle final {
    FloatRect rect{330.0, 578.0, 100.0, 15.0};
    double speed{520.0};
};

struct Ball final {
    double x{380.0};
    double y{548.0};
    double radius{7.0};
    double velocityX{210.0};
    double velocityY{-310.0};
};

struct Brick final {
    FloatRect rect{};
    int hitPoints{1};
    int score{50};
    gamecore::Color color{240, 240, 240, 255};

    [[nodiscard]] auto alive() const -> bool { return hitPoints > 0; }
};

class ArkanoidGame final : public gamecore::IGame {
public:
    explicit ArkanoidGame(gamecore::EventBus& events);

    [[nodiscard]] auto name() const -> std::string_view override;
    void restart() override;
    void handleInput(const gamecore::InputFrame& input) override;
    void update(gamecore::Seconds deltaTime) override;
    void render(gamecore::IRenderer2D& renderer) const override;
    [[nodiscard]] auto wantsToQuit() const -> bool override;

private:
    static constexpr int ScreenWidth = 760;
    static constexpr int ScreenHeight = 640;
    static constexpr int WallLeft = 38;
    static constexpr int WallRight = 722;
    static constexpr int WallTop = 42;

    gamecore::EventBus& events_;
    Paddle paddle_{};
    Ball ball_{};
    std::vector<Brick> bricks_;
    ArkanoidPhase phase_{ArkanoidPhase::Ready};
    int score_{};
    int lives_{3};
    bool quit_{};
    int moveDirection_{};

    void resetRound();
    void buildLevel();
    void launchBall();
    void updatePaddle(double dt);
    void updateBall(double dt);
    void resolvePaddleCollision();
    void resolveBrickCollisions();
    void loseLife();
    [[nodiscard]] auto aliveBrickCount() const -> int;

    static auto toRect(FloatRect rect) -> gamecore::Recti;
    static auto ballRect(const Ball& ball) -> gamecore::Recti;
};

} // namespace games::arkanoid
