#pragma once

#include <array>
#include <memory>
#include <string_view>
#include <vector>

#include "gamecore/Game.hpp"
#include "gamecore/GameRegistry.hpp"

namespace gamecore {

class GameShell final : public IGame {
public:
    explicit GameShell(const GameRegistry& registry);

    [[nodiscard]] auto name() const -> std::string_view override;
    void restart() override;
    void handleInput(const InputFrame& input) override;
    void update(Seconds deltaTime) override;
    void render(IRenderer2D& renderer) const override;
    [[nodiscard]] auto wantsToQuit() const -> bool override;

private:
    struct Star final {
        double x{};
        double y{};
        double z{};
        double speed{};
        int size{};
    };

    static constexpr int ScreenWidth = 760;
    static constexpr int ScreenHeight = 640;
    static constexpr int StarCount = 140;

    const GameRegistry& registry_;
    std::vector<GameDescriptor> games_;
    std::unique_ptr<IGame> activeGame_;
    std::array<Star, StarCount> stars_{};
    int selectedIndex_{};
    bool quit_{};

    void launchSelectedGame();
    void returnToMenu();
    void resetStars();
    void updateStars(Seconds deltaTime);
    void renderMenu(IRenderer2D& renderer) const;
    void renderStarfield(IRenderer2D& renderer) const;
};

} // namespace gamecore
