#ifndef FOOLGO_PLAYER_H
#define FOOLGO_PLAYER_H

#include "../board/def.h"
#include "../board/position.h"

using foolgo::board::Position;

namespace foolgo {

namespace board {
template <board::BoardLen BOARD_LEN>
class FullBoard;
} /* namespace board */

namespace player {

using PlayerType = int8_t;
const PlayerType BLACK_PLAYER = 0;
const PlayerType WHITE_PLAYER = 1;
const PlayerType NO_PLAYER = 2; 

template<board::BoardLen BOARD_LEN>
class Player {
  public:
    virtual ~Player() = default;
    virtual board::PositionIndex playMove(const board::FullBoard<BOARD_LEN> &full_board) = 0;

  private:
};
} // namespace player
} // namespace foolgo

#endif