#ifndef FOOLGO_POSITION_H
#define FOOLGO_POSITION_H

#include <iostream>
#include <string>

#include "def.h"

namespace foolgo {
namespace board {

struct Position {
    Position() : x(0), y(0) {}
    Position(BoardLen x, BoardLen y) {
        set(x, y);
    }

    void set(BoardLen x, BoardLen y) {
        this->x = x;
        this->y = y;
    }

    std::string toString() const {
        return "[" + std::to_string(x) + "," + std::to_string(y) + "]";
    }

    BoardLen x, y;
};

Position GetAdjacentPosition(Position postion, BoardLen i);
Position GetObliquePosition(Position postion, BoardLen i);

template <BoardLen BOARD_LEN>
constexpr BoardLen BoardLenMinusOne() {
    return BOARD_LEN - 1;
}

template <BoardLen BOARD_LEN>
constexpr int16_t MoveCountUpperBoundPerGame() {
    return BoardLenSquare<BOARD_LEN>() * 4;
}

} // namespace board
} // namespace foolgo

#endif