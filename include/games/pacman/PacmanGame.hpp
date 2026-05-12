#pragma once

#include <array>
#include <string_view>
#include <vector>

#include "gamecore/EventBus.hpp"
#include "gamecore/Game.hpp"
#include "gamecore/Geometry.hpp"

namespace games::pacman {

enum class PacmanPhase {
    Ready,
    Playing,
    Paused,
    Won,
    GameOver
};

enum class Direction {
    None,
    Left,
    Right,
    Up,
    Down
};

struct Actor final {
    double x{};
    double y{};
    Direction direction{Direction::None};
    Direction requested{Direction::None};
    double speed{};
};

struct Ghost final {
    Actor actor{};
    gamecore::Color color{};
};

class PacmanGame final : public gamecore::IGame {
public:
    explicit PacmanGame(gamecore::EventBus& events);

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
    static constexpr int MazeWidth = 19;
    static constexpr int MazeHeight = 21;
    static constexpr int MazeOffsetX = 152;
    static constexpr int MazeOffsetY = 72;

    using MazeRows = std::array<std::string_view, MazeHeight>;

    gamecore::EventBus& events_;
    std::array<std::string, MazeHeight> pellets_{};
    Actor pacman_{};
    std::vector<Ghost> ghosts_;
    PacmanPhase phase_{PacmanPhase::Ready};
    int score_{};
    int lives_{3};
    int remainingPellets_{};
    bool quit_{};

    void resetRound();
    void resetPellets();
    void updatePacman(double dt);
    void updateGhosts(double dt);
    void moveActorAlongGrid(Actor& actor, double dt);
    void collectPellet();
    void resolveGhostCollision();
    void loseLife();

    [[nodiscard]] auto canMoveFrom(double x, double y, Direction direction) const -> bool;
    [[nodiscard]] auto isWallCell(int col, int row) const -> bool;
    [[nodiscard]] auto currentCol(double x) const -> int;
    [[nodiscard]] auto currentRow(double y) const -> int;
    [[nodiscard]] auto centeredOnTile(double value, int offset) const -> bool;
    [[nodiscard]] auto tileCenterX(int col) const -> double;
    [[nodiscard]] auto tileCenterY(int row) const -> double;

    static auto directionVector(Direction direction) -> gamecore::Vec2i;
    static auto actorRect(const Actor& actor, int size) -> gamecore::Recti;
    static auto opposite(Direction direction) -> Direction;
    static auto mazeRows() -> const MazeRows&;
};

} // namespace games::pacman
