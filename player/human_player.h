#ifndef FOOLGO_HUMAN_PLAYER_H
#define FOOLGO_HUMAN_PLAYER_H

#include "player.h"
#include "../board/pos_ctrl.h"

namespace foolgo {

namespace player {
template <board::BoardLen BOARD_LEN>
class HumanPlayer : public Player<BOARD_LEN> {
  public:
    board::PositionIndex playMove(const board::FullBoard<BOARD_LEN> &full_board);

  private:
};

template <board::BoardLen BOARD_LEN>
board::PositionIndex HumanPlayer<BOARD_LEN>::playMove(const board::FullBoard<BOARD_LEN> &full_board) {
    board::Position postion;
    std::cout << "(x, y)" << std::endl;
    std::cin >> postion.x >> postion.y;
    return board::PstionAndIndxCtrl<BOARD_LEN>::getInstance().getIndex(postion);
}
} // namespace player
} // namespace foolgo

#endif