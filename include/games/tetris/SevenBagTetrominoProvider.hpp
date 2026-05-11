#pragma once

#include <array>
#include <random>
#include <vector>

#include "games/tetris/ITetrominoProvider.hpp"

namespace games::tetris {

class SevenBagTetrominoProvider final : public ITetrominoProvider {
public:
    SevenBagTetrominoProvider();
    explicit SevenBagTetrominoProvider(std::uint32_t seed);

    [[nodiscard]] auto next() -> TetrominoKind override;

private:
    std::mt19937 rng_;
    std::vector<TetrominoKind> bag_;

    void refillBag();
};

} // namespace games::tetris
