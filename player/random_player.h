#ifndef FOOLGO_SRC_PLAYER_RANDOM_PLAYER_H_
#define FOOLGO_SRC_PLAYER_RANDOM_PLAYER_H_

#include <cassert>
#include <cstdint>

#include "../board/full_board.h"
#include "../board/position.h"
#include "../utility/math.h"
#include "player.h"

namespace foolgo {
namespace player {

template <board::BoardLen BOARD_LEN>
class RandomPlayer : public Player<BOARD_LEN> {
  public:
    explicit RandomPlayer(uint32_t seed) : seed_(seed) {}

  protected:
    board::PositionIndex playMove(const board::FullBoard<BOARD_LEN> &full_board);

  private:
    uint32_t seed_;
};

template <board::BoardLen BOARD_LEN>
board::PositionIndex RandomPlayer<BOARD_LEN>::playMove(const board::FullBoard<BOARD_LEN> &full_board) {
    auto playable_indexes = full_board.playableIndexes(full_board.getMovePlayer());
    assert(!playable_indexes.empty());

    board::PositionIndex rand = math::Rand(playable_indexes.size() - 1, seed_);
    return playable_indexes.at(rand);
}

} /* namespace player */
} /* namespace foolgo */

#endif