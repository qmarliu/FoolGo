#ifndef FOOLGO_SRC_PIECE_STRUCTRUE_EYE_SET_H_
#define FOOLGO_SRC_PIECE_STRUCTRUE_EYE_SET_H_

#include <bitset>

#include "position.h"
#include "def.h"
#include "../utility/math.h"

namespace foolgo {
namespace board {

template <BoardLen BOARD_LEN>
class EyeSet {
  public:
    EyeSet() = default;
    ~EyeSet() = default;
    void copy(const EyeSet &es);

    void setEye(PositionIndex indx, bool v) {
        eyes_[indx] = v;
    }
    void setRealEye(PositionIndex indx, bool v) {
        real_eyes_[indx] = v;
    }
    bool isEye(PositionIndex indx) const {
        return eyes_[indx];
    }
    bool isRealEye(PositionIndex indx) const {
        return real_eyes_[indx];
    }
    PositionIndex realCount() const {
        return real_eyes_.count();
    }

    std::vector<board::PositionIndex> getRealEyes() const {
        return math::GetOnePositionIndexes<board::BoardLenSquare<BOARD_LEN>()>(real_eyes_);
    }

  private:
    BitSet<BOARD_LEN> eyes_;
    BitSet<BOARD_LEN> real_eyes_;

    DISALLW_COPY_AND_ASSIGNE_AND_MOVE(EyeSet);
};

template <BoardLen BOARD_LEN>
bool isFakeEye(const EyeSet<BOARD_LEN> &eyeset, PositionIndex indx) {
    return eyeset.isEye(indx) && !eyeset.isRealEye(indx);
}

template <BoardLen BOARD_LEN>
inline void EyeSet<BOARD_LEN>::copy(const EyeSet &es) {
    eyes_ = es.eyes_;
    real_eyes_ = es.real_eyes_;
}

} // namespace piece_structure
} // namespace foolgo

#endif
