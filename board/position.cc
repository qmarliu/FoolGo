#include "position.h"

namespace foolgo {
namespace board {
    
std::ostream &operator<<(std::ostream &os, const Position &position) {
    return os << position.toString();
}

const BoardLen STRAIGHT_ORNTTIONS[4][2] = { { 0, -1 }, { 1, 0 }, { 0,
    1 }, { -1, 0 } };

const BoardLen OBLIQUE_ORNTTIONS[4][2] = { { 1, -1 }, { 1, 1 }, { -1,
    1 }, { -1, -1 } };

Position GetAdjacentPosition(Position postion, BoardLen i)
{
    return Position(postion.x + STRAIGHT_ORNTTIONS[i][0],
                    postion.y + STRAIGHT_ORNTTIONS[i][1]);
}

Position GetObliquePosition(Position postion, BoardLen i)
{
    return Position(postion.x + OBLIQUE_ORNTTIONS[i][0],
                    postion.y + OBLIQUE_ORNTTIONS[i][1]);
}
} // namespace board
} // namespace foolgo