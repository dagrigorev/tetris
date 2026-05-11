#include "games/tetris/SevenBagTetrominoProvider.hpp"

#include <algorithm>
#include <random>

namespace games::tetris {

SevenBagTetrominoProvider::SevenBagTetrominoProvider()
    : SevenBagTetrominoProvider(std::random_device{}()) {}

SevenBagTetrominoProvider::SevenBagTetrominoProvider(const std::uint32_t seed)
    : rng_(seed) {
    refillBag();
}

auto SevenBagTetrominoProvider::next() -> TetrominoKind {
    if (bag_.empty()) {
        refillBag();
    }

    const auto result = bag_.back();
    bag_.pop_back();
    return result;
}

void SevenBagTetrominoProvider::refillBag() {
    bag_ = {
        TetrominoKind::I,
        TetrominoKind::O,
        TetrominoKind::T,
        TetrominoKind::S,
        TetrominoKind::Z,
        TetrominoKind::J,
        TetrominoKind::L
    };

    std::ranges::shuffle(bag_, rng_);
}

} // namespace games::tetris
