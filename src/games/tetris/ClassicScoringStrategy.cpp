#include "games/tetris/ScoringStrategy.hpp"

#include <array>
#include <algorithm>

namespace games::tetris {

void ClassicScoringStrategy::applySoftDrop(ScoreState& state) const {
    state.score += 1;
}

void ClassicScoringStrategy::applyHardDrop(ScoreState& state, const int cells) const {
    state.score += cells * 2;
}

void ClassicScoringStrategy::applyLineClear(ScoreState& state, const int clearedLines) const {
    if (clearedLines <= 0) {
        return;
    }

    static constexpr std::array<int, 5> points{0, 100, 300, 500, 800};
    const auto safeLines = std::clamp(clearedLines, 0, 4);
    state.score += points[static_cast<std::size_t>(safeLines)] * state.level;
    state.lines += clearedLines;
    state.level = 1 + state.lines / 10;
}

} // namespace games::tetris
