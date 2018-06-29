#ifndef FOOLGO_BOARD_H
#define FOOLGO_BOARD_H

#include <cassert>
#include <string>
#include <cstring>

#include "def.h"
// #include "../utility/common.h"
#include "pos_ctrl.h"
#include "position.h"

namespace foolgo {
namespace board {

template <BoardLen BOARD_LEN>
class Board {
  public:
    Board() {
        memset(points_, EMPTY_POINT, sizeof(points_));
    }
    virtual ~Board() {}
    
    PointState getPoint(PositionIndex index) const;
    PointState getPoint(const Position &position) const;

    void setPoint(PositionIndex index, PointState state);
    void setPoint(const Position &position, PointState state);

    std::string toString() const
    {
        std::string boardString = "  ";
        for (BoardLen i = 0; i < BOARD_LEN; ++i) {
            BoardLen num = i % 10;
            boardString += (std::to_string(i) + "  ");
        }

        for (BoardLen y = 0; y < BOARD_LEN; ++y) {
            boardString += "\n";
            boardString += std::to_string(y % 10) + " ";
            for (BoardLen x = 0; x < BOARD_LEN; ++x) {
                boardString += getPointStateChar(POSITION_TO_INDEX(Position(x, y)));
                boardString += "  ";
            }
        }
        return boardString;
    }

  private:
    char getPointStateChar(PositionIndex index) const
    {
        assert(INDEX_IS_IN_BOARD(index));
        if (points_[index] == EMPTY_POINT) {
            return EMPTY_OUTPUT;
        }
        else if (points_[index] == BLACK_POINT) {
            return BLACK_OUTPUT;
        }
        else if (points_[index] == WHITE_POINT) {
            return WHITE_OUTPUT;
        }
        assert(false);
    }

    PointState points_[BoardLenSquare<BOARD_LEN>()];
    // DISALLW_COPY_AND_ASSIGNE_AND_MOVE(Board);
};

template <BoardLen BOARD_LEN>
inline PointState Board<BOARD_LEN>::getPoint(PositionIndex index) const {
    assert(INDEX_IS_IN_BOARD(index));
    return points_[index];
}

template <BoardLen BOARD_LEN>
inline PointState Board<BOARD_LEN>::getPoint(const Position &position) const {
    return getPoint(POSITION_TO_INDEX(position));
}

template <BoardLen BOARD_LEN>
inline void Board<BOARD_LEN>::setPoint(PositionIndex index, PointState state) {
    assert(INDEX_IS_IN_BOARD(index));
    points_[index] = state;
}

template <BoardLen BOARD_LEN>
inline void Board<BOARD_LEN>::setPoint(const Position &position, PointState state) {
    setPoint(POSITION_TO_INDEX(position), state);
}

template <BoardLen BOARD_LEN>
std::ostream &operator<<(std::ostream &os, const Board<BOARD_LEN> &board) {
    return os << board.toString();
}
} // namespace board
} // namespace foolgo
#endif