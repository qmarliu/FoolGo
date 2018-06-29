#ifndef FOOLGO_POS_CTRL_H
#define FOOLGO_POS_CTRL_H

#include "def.h"
// #include "../utility/common.h"
#include "position.h"
#include <iostream>
namespace foolgo {
namespace board {

template <BoardLen BOARD_LEN>
class PstionAndIndxCtrl {
  public:
    typedef enum {
        IN_CENTRAL = 0,
        IN_EDGE = 1,
        IN_CORNER = 2,
    } CentralEdgeCorner;

    static PstionAndIndxCtrl<BOARD_LEN> &getInstance();
    Position getPosition(PositionIndex index) const {
        return position_[index];
    }

    PositionIndex getIndex(const Position &position) const {
        return indexes_[position.x][position.y];
    }

    bool isInBoard(const Position &pos) const {
        return pos.x >= 0 && pos.x < BOARD_LEN && pos.y >= 0 && pos.y < BOARD_LEN;
    }

    bool isInBoard(PositionIndex index) const {
        isInBoard(getPosition(index));
    }
    CentralEdgeCorner centralOrEdgeOrCorner(const Position &pos);

  private:
    Position position_[BoardLenSquare<BOARD_LEN>()];
    PositionIndex indexes_[BOARD_LEN][BOARD_LEN];
    PstionAndIndxCtrl() {
        for (int x = 0; x < BOARD_LEN; ++x) {
            for (int y = 0; y < BOARD_LEN; ++y) {
                PositionIndex index = y * BOARD_LEN + x;
                indexes_[x][y] = index;
                position_[index].set(x, y);
            }
        }
    }
    DISALLW_COPY_AND_ASSIGNE_AND_MOVE(PstionAndIndxCtrl);
};

template <BoardLen BOARD_LEN>
inline PstionAndIndxCtrl<BOARD_LEN> &PstionAndIndxCtrl<BOARD_LEN>::getInstance() {
    static PstionAndIndxCtrl<BOARD_LEN> ins;
    return ins;
}

template <BoardLen BOARD_LEN>
typename PstionAndIndxCtrl<BOARD_LEN>::CentralEdgeCorner 
PstionAndIndxCtrl<BOARD_LEN>::centralOrEdgeOrCorner(const Position &pos) {
    if ((pos.x == 0 && pos.y == 0) || (pos.x == 0 && pos.y == BoardLenMinusOne<BOARD_LEN>()) || (pos.x == BoardLenMinusOne<BOARD_LEN>() && pos.y == 0) || (pos.x == BoardLenMinusOne<BOARD_LEN>() && pos.y == BoardLenMinusOne<BOARD_LEN>())) {
        return IN_CORNER;
    } else if (pos.x > 0 && pos.x < BoardLenMinusOne<BOARD_LEN>() && pos.y > 0 && pos.y < BoardLenMinusOne<BOARD_LEN>()) {
        return IN_CENTRAL;
    } else {
        return IN_EDGE;
    }
}

#define POSITION_TO_INDEX(position) PstionAndIndxCtrl<BOARD_LEN>::getInstance().getIndex(position)
#define INDEX_TO_POSITION(index) PstionAndIndxCtrl<BOARD_LEN>::getInstance().getPosition(index)
#define POSITION_IS_IN_BOARD(position) PstionAndIndxCtrl<BOARD_LEN>::getInstance().isInBoard(position)
#define INDEX_IS_IN_BOARD(index) PstionAndIndxCtrl<BOARD_LEN>::getInstance().isInBoard(index)

} // namespace board
} // namespace foolgo

#endif