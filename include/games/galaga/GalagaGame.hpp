#pragma once

#include <string_view>
#include <vector>

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"
#include "gamecore/Geometry.hpp"

namespace games::galaga {

enum class GalagaPhase {
    Ready,
    Playing,
    Paused,
    WaveCleared,
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

struct PlayerShip final {
    FloatRect rect{360.0, 568.0, 40.0, 30.0};
    double speed{360.0};
};

struct Enemy final {
    FloatRect rect{};
    int row{};
    int score{100};
    bool alive{true};
};

struct Shot final {
    FloatRect rect{};
    double velocityY{};
    bool fromPlayer{};
    bool active{true};
};

class GalagaGame final : public gamecore::IGame {
public:
    explicit GalagaGame(gamecore::EventBus& events);

    [[nodiscard]] auto name() const -> std::string_view override;
    void restart() override;
    void handleInput(const gamecore::InputFrame& input) override;
    void update(gamecore::Seconds deltaTime) override;
    void render(gamecore::IRenderer2D& renderer) const override;
    [[nodiscard]] auto wantsToQuit() const -> bool override;

private:
    static constexpr int ScreenWidth = 760;
    static constexpr int ScreenHeight = 640;
    static constexpr int PlayLeft = 36;
    static constexpr int PlayRight = 724;
    static constexpr int PlayTop = 42;
    static constexpr int PlayBottom = 610;

    gamecore::EventBus& events_;
    PlayerShip player_{};
    std::vector<Enemy> enemies_;
    std::vector<Shot> shots_;
    GalagaPhase phase_{GalagaPhase::Ready};
    int score_{};
    int lives_{3};
    int wave_{1};
    int moveDirection_{};
    double enemyDirection_{1.0};
    double enemyFireTimer_{0.9};
    double waveMessageTimer_{};
    bool quit_{};

    void buildWave();
    void updatePlayer(double dt);
    void updateEnemies(double dt);
    void updateShots(double dt);
    void firePlayerShot();
    void fireEnemyShot();
    void resolveCollisions();
    void loseLife();
    [[nodiscard]] auto aliveEnemyCount() const -> int;

    static auto toRect(FloatRect rect) -> gamecore::Recti;
};

} // namespace games::galaga
