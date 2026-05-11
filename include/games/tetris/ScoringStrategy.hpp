#pragma once

namespace games::tetris {

struct ScoreState final {
    int score{};
    int lines{};
    int level{1};
};

class IScoringStrategy {
public:
    virtual ~IScoringStrategy() = default;
    virtual void applySoftDrop(ScoreState& state) const = 0;
    virtual void applyHardDrop(ScoreState& state, int cells) const = 0;
    virtual void applyLineClear(ScoreState& state, int clearedLines) const = 0;
};

class ClassicScoringStrategy final : public IScoringStrategy {
public:
    void applySoftDrop(ScoreState& state) const override;
    void applyHardDrop(ScoreState& state, int cells) const override;
    void applyLineClear(ScoreState& state, int clearedLines) const override;
};

} // namespace games::tetris
