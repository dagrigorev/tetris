#include "gamecore/GameShell.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace gamecore {
namespace {

[[nodiscard]] auto wrapIndex(int value, const int count) -> int {
    if (count <= 0) {
        return 0;
    }
    if (value < 0) {
        return count - 1;
    }
    if (value >= count) {
        return 0;
    }
    return value;
}

[[nodiscard]] auto starBrightness(const double z) -> Color {
    const auto intensity = static_cast<std::uint8_t>(std::clamp(255.0 - z * 1.7, 70.0, 255.0));
    return {intensity, intensity, static_cast<std::uint8_t>(std::min(255, static_cast<int>(intensity) + 20)), 255};
}

} // namespace

GameShell::GameShell(const GameRegistry& registry) : registry_(registry), games_(registry_.availableGames()) {
    if (games_.empty()) {
        throw std::runtime_error("No games registered");
    }
    resetStars();
}

auto GameShell::name() const -> std::string_view {
    return "Layered Games";
}

void GameShell::restart() {
    returnToMenu();
    selectedIndex_ = 0;
    quit_ = false;
    resetStars();
}

void GameShell::handleInput(const InputFrame& input) {
    using enum InputCommand;

    if (input.isPressed(Quit)) {
        quit_ = true;
        return;
    }

    if (activeGame_ != nullptr) {
        if (input.isPressed(Back)) {
            returnToMenu();
            return;
        }

        activeGame_->handleInput(input);
        if (activeGame_->wantsToQuit()) {
            returnToMenu();
        }
        return;
    }

    const auto count = static_cast<int>(games_.size());
    if (input.isPressed(Back)) {
        quit_ = true;
        return;
    }
    if (input.isPressed(MenuUp)) {
        selectedIndex_ = wrapIndex(selectedIndex_ - 1, count);
    }
    if (input.isPressed(MenuDown)) {
        selectedIndex_ = wrapIndex(selectedIndex_ + 1, count);
    }
    if (input.isPressed(Select) || input.isPressed(HardDrop)) {
        launchSelectedGame();
    }
}

void GameShell::update(const Seconds deltaTime) {
    updateStars(deltaTime);

    if (activeGame_ != nullptr) {
        activeGame_->update(deltaTime);
        if (activeGame_->wantsToQuit()) {
            returnToMenu();
        }
    }
}

void GameShell::render(IRenderer2D& renderer) const {
    if (activeGame_ != nullptr) {
        activeGame_->render(renderer);
        return;
    }

    renderMenu(renderer);
}

auto GameShell::wantsToQuit() const -> bool {
    return quit_;
}

void GameShell::launchSelectedGame() {
    if (games_.empty()) {
        return;
    }
    activeGame_ = registry_.create(games_.at(static_cast<std::size_t>(selectedIndex_)).id);
}

void GameShell::returnToMenu() {
    activeGame_.reset();
}

void GameShell::resetStars() {
    for (std::size_t i = 0; i < stars_.size(); ++i) {
        const auto seed = static_cast<int>(i) + 1;
        const auto x = static_cast<double>((seed * 97) % 1600 - 800) / 10.0;
        const auto y = static_cast<double>((seed * 193) % 1200 - 600) / 10.0;
        const auto z = static_cast<double>((seed * 53) % 120 + 20);
        stars_[i] = Star{.x = x, .y = y, .z = z, .speed = 28.0 + static_cast<double>((seed * 17) % 90), .size = 1 + (seed % 3)};
    }
}

void GameShell::updateStars(const Seconds deltaTime) {
    const auto dt = std::clamp(deltaTime.count(), 0.0, 0.05);
    for (auto& star : stars_) {
        star.z -= star.speed * dt;
        if (star.z <= 4.0) {
            star.z = 140.0;
            star.x = -80.0 + std::fmod((star.x * 37.0 + star.speed * 11.0), 160.0);
            star.y = -60.0 + std::fmod((star.y * 41.0 + star.speed * 7.0), 120.0);
        }
    }
}

void GameShell::renderMenu(IRenderer2D& renderer) const {
    renderer.beginFrame({2, 4, 14, 255});
    renderStarfield(renderer);

    const Recti titlePanel{112, 82, 536, 112};
    renderer.fillRect(titlePanel, {8, 12, 30, 215});
    renderer.drawRect(titlePanel, {80, 112, 180, 255});
    renderer.drawText({174, 112}, "LAYERED GAMES", {236, 244, 255, 255}, 4);
    renderer.drawText({222, 166}, "SELECT A GAME", {130, 180, 255, 255}, 2);

    const Recti menuPanel{170, 242, 420, 210};
    renderer.fillRect(menuPanel, {8, 12, 28, 225});
    renderer.drawRect(menuPanel, {74, 98, 160, 255});

    for (std::size_t i = 0; i < games_.size(); ++i) {
        const auto y = 284 + static_cast<int>(i) * 58;
        const bool selected = static_cast<int>(i) == selectedIndex_;
        const Recti item{204, y - 12, 352, 42};
        renderer.fillRect(item, selected ? Color{28, 55, 110, 230} : Color{13, 18, 40, 190});
        renderer.drawRect(item, selected ? Color{140, 190, 255, 255} : Color{48, 65, 100, 255});
        renderer.drawText({226, y}, selected ? ">" : "-", selected ? Color{190, 225, 255, 255} : Color{90, 110, 145, 255}, 2);
        renderer.drawText({270, y}, games_[i].displayName, selected ? Color{245, 250, 255, 255} : Color{180, 195, 220, 255}, 2);
    }

    renderer.drawText({176, 502}, "UP DOWN OR W S - SELECT", {150, 170, 205, 255}, 1);
    renderer.drawText({176, 522}, "ENTER OR SPACE - START", {150, 170, 205, 255}, 1);
    renderer.drawText({176, 542}, "ESC - BACK TO MENU OR EXIT", {150, 170, 205, 255}, 1);

    renderer.endFrame();
}

void GameShell::renderStarfield(IRenderer2D& renderer) const {
    const Vec2i center{ScreenWidth / 2, ScreenHeight / 2};

    for (const auto& star : stars_) {
        const auto perspective = 180.0 / star.z;
        const auto x = center.x + static_cast<int>(star.x * perspective);
        const auto y = center.y + static_cast<int>(star.y * perspective);

        if (x < 0 || x >= ScreenWidth || y < 0 || y >= ScreenHeight) {
            continue;
        }

        const auto tail = std::max(1, static_cast<int>((140.0 - star.z) / 22.0));
        const auto color = starBrightness(star.z);
        renderer.drawLine({x - tail, y - tail}, {x, y}, color);
        renderer.fillRect({x, y, star.size, star.size}, color);
    }

    renderer.drawLine({0, ScreenHeight / 2}, {ScreenWidth, ScreenHeight / 2}, {22, 34, 70, 90});
    renderer.drawLine({ScreenWidth / 2, 0}, {ScreenWidth / 2, ScreenHeight}, {22, 34, 70, 70});
}

} // namespace gamecore
