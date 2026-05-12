#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"
#include "gamecore/Geometry.hpp"

namespace games::battle_city {

enum class BattlePhase { Ready, Playing, Paused, StageCleared, GameOver };
enum class Tile : std::uint8_t { Empty, Brick, Steel, Water, Trees, Base, DestroyedBase };
enum class Direction : std::uint8_t { Up, Right, Down, Left };

struct Vec2d final { double x{}; double y{}; };

struct Tank final {
    Vec2d position{};
    Direction direction{Direction::Up};
    double speed{120.0};
    bool player{false};
    bool alive{true};
    double fireCooldown{};
    double aiTimer{};
    int hp{1};
    std::uint8_t spriteKind{0};
};

struct Bullet final {
    Vec2d position{};
    Direction direction{Direction::Up};
    bool fromPlayer{false};
    bool active{true};
    double speed{330.0};
};

struct Explosion final {
    Vec2d position{};
    double age{};
    double duration{0.45};
};

class BattleCityGame final : public gamecore::IGame {
public:
    explicit BattleCityGame(gamecore::EventBus& events);

    [[nodiscard]] auto name() const -> std::string_view override;
    void restart() override;
    void handleInput(const gamecore::InputFrame& input) override;
    void update(gamecore::Seconds deltaTime) override;
    void render(gamecore::IRenderer2D& renderer) const override;
    [[nodiscard]] auto wantsToQuit() const -> bool override;

private:
    static constexpr int ScreenWidth = 760;
    static constexpr int ScreenHeight = 640;
    static constexpr int TileSize = 24;
    static constexpr int BoardCols = 26;
    static constexpr int BoardRows = 22;
    static constexpr int BoardLeft = 68;
    static constexpr int BoardTop = 58;
    static constexpr int TankSize = 22;
    static constexpr int BulletSize = 6;

    gamecore::EventBus& events_;
    std::array<Tile, BoardCols * BoardRows> tiles_{};
    Tank player_{};
    std::vector<Tank> enemies_;
    std::vector<Bullet> bullets_;
    std::vector<Explosion> explosions_;
    BattlePhase phase_{BattlePhase::Ready};
    Direction requestedDirection_{Direction::Up};
    bool hasMoveInput_{};
    bool fireRequested_{};
    bool quit_{};
    int score_{};
    int lives_{3};
    int stage_{1};
    int enemiesRemaining_{16};
    int spawnCursor_{};
    double spawnTimer_{};
    double messageTimer_{};
    double animationTime_{};

    void buildStage();
    void spawnEnemy();
    void updatePlayer(double dt);
    void updateEnemies(double dt);
    void updateBullets(double dt);
    void fire(Tank& tank, bool fromPlayer);
    void resolveBullet(Bullet& bullet);
    void loseLife();
    void clearStage();
    void destroyBase();

    [[nodiscard]] auto canMoveTo(Vec2d position, bool playerTank) const -> bool;
    [[nodiscard]] auto tileAt(int col, int row) const -> Tile;
    void setTile(int col, int row, Tile tile);
    [[nodiscard]] auto worldRect(Vec2d position, int size) const -> gamecore::Recti;
    [[nodiscard]] static auto directionVector(Direction direction) -> Vec2d;
    [[nodiscard]] static auto intersects(gamecore::Recti a, gamecore::Recti b) -> bool;
    [[nodiscard]] static auto toTileCoord(double world, int origin) -> int;

    void addExplosion(Vec2d position, double duration = 0.45);
    void drawTile(gamecore::IRenderer2D& renderer, int col, int row, Tile tile) const;
    void drawTank(gamecore::IRenderer2D& renderer, const Tank& tank) const;
    void drawExplosion(gamecore::IRenderer2D& renderer, const Explosion& explosion) const;
    void drawBase(gamecore::IRenderer2D& renderer, int col, int row, bool destroyed) const;
};

} // namespace games::battle_city
