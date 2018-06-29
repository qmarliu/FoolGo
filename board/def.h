#ifndef FOOLGO_MACRO_H
#define FOOLGO_MACRO_H
#include <cstdint>
#include <bitset>

namespace foolgo {
namespace board {

using BoardLen = int16_t;
constexpr BoardLen MAIN_BOARD_LEN = 9;

using PointState = int8_t;
const PointState BLACK_POINT = 0;
const PointState WHITE_POINT = 1;
const PointState EMPTY_POINT = 2;

using PositionIndex = int16_t;

template <BoardLen BOARD_LEN>
constexpr PositionIndex BoardLenSquare() {
    return BOARD_LEN * BOARD_LEN;
}

template <BoardLen BOARD_LEN>
using BitSet = std::bitset<BoardLenSquare<BOARD_LEN>()>;

const char BLACK_OUTPUT = 'x';
const char LAST_MOVE_BLACK_OUTPUT = 'X';
const char WHITE_OUTPUT = 'o';
const char LAST_MOVE_WHITE_OUTPUT = 'O';
const char EMPTY_OUTPUT = '.';
const char LAST_MOVE_EMPTY_OUTPUT = '*';

} // namespace board
} // namespace foolgo
#endif