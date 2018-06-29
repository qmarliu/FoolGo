
#ifndef FOOLGO_FULL_BOARD_H
#define FOOLGO_FULL_BOARD_H

#include <array>
#include <bitset>
#include <stdexcept>
#include <vector>

#include "../player/player.h"
#include "board.h"
#include "chain_set.h"
#include "eye_set.h"

using foolgo::player::BLACK_PLAYER;
using foolgo::player::NO_PLAYER;
using foolgo::player::PlayerType;
using foolgo::player::WHITE_PLAYER;
using std::vector;

namespace foolgo {
namespace board {
template <BoardLen BOARD_LEN>
class FullBoard : public Board<BOARD_LEN> {
  public:
    FullBoard();
    void playMove(PositionIndex playIndex);
    bool isEnd();
    std::vector<PositionIndex> playableIndexes(PlayerType type) const;

    PlayerType getMovePlayer() const {
        return current_player_;
    };

    PlayerType oppositePlayer() {
        return (current_player_ == BLACK_PLAYER) ? WHITE_PLAYER : BLACK_PLAYER;
    }

    PlayerType oppositePlayer(PlayerType type) {
        return (type == BLACK_PLAYER) ? WHITE_PLAYER : BLACK_PLAYER;
    }

    PointState getPointState(PositionIndex indx) const {
        return Board<BOARD_LEN>::getPoint(indx);
    }

    PointState getPointState(const Position &pos) const {
        return Board<BOARD_LEN>::getPoint(pos);
    }

    vector<PositionIndex> removeChain(PlayerType force, PositionIndex indx);

    std::string toString() const {
        return Board<BOARD_LEN>::toString();
    }

  private:
    void setSpecifiedAirForAdjacentChains(PositionIndex indx, bool v);
    void modifyEyesStateAndObliqueRealEyesState(PositionIndex indx);
    bool isSelfPieceOrEye(PlayerType force, PositionIndex indx) const;
    void modifyRealEyesState(PlayerType force, PositionIndex indx);
    void setRealEyeAsTrue(PlayerType force, PositionIndex indx);
    void modifyRealEyesPlayableState();
    std::array<std::bitset<BoardLenSquare<BOARD_LEN>()>, 2> playerable_index_;
    PlayerType current_player_;

    std::array<ChainSet<BOARD_LEN>, 2> chain_sets_;
    std::array<EyeSet<BOARD_LEN>, 2> eye_states_array_; //眼
    PositionIndex ko_indx_; //劫的位置
};

template <BoardLen BOARD_LEN>
std::ostream &operator<<(std::ostream &os, const FullBoard<BOARD_LEN> &board) {
    return os << board.toString();
}

template <BoardLen BOARD_LEN>
FullBoard<BOARD_LEN>::FullBoard() {
    playerable_index_[BLACK_PLAYER].set();
    playerable_index_[WHITE_PLAYER].set();

    current_player_ = BLACK_PLAYER; //黑先
    ko_indx_ = -1;
}

template <BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::playMove(PositionIndex playIndex) {
    if (!INDEX_IS_IN_BOARD(playIndex)) {
        throw std::out_of_range("输入超出了范围，请重新输入：");
    }

    Position playPosition = INDEX_TO_POSITION(playIndex);

    if (!playerable_index_[current_player_].test(playIndex) || 
    !playerable_index_[oppositePlayer()].test(playIndex) ){
        throw std::out_of_range("已经有棋子在这个位置，请重新输入:");
    }

    if (ko_indx_ == playIndex) {
        throw std::out_of_range("这个位置是劫，请重新输入:");
    }

    for (int i = 0; i < 2; ++i) {
        playerable_index_[i].reset(playIndex);
        eye_states_array_[i].setEye(playIndex, false);
        eye_states_array_[i].setRealEye(playIndex, false);
    }

    std::vector<PositionIndex> ate_piecies_indexes[4];
    std::vector<PositionIndex> suicided_pieces_indexes;
    //检查下子的地方，会不会导致相邻的对方棋子的气为0
    std::bitset<BoardLenSquare<BOARD_LEN>()> air_set;
    for (BoardLen i = 0; i < 4; ++i) {
        Position adjacentPosition = GetAdjacentPosition(playPosition, i);
        if (!POSITION_IS_IN_BOARD(adjacentPosition)) {
            continue;
        }
        PositionIndex  adjacentIndex = POSITION_TO_INDEX(adjacentPosition);

        if (getPointState(adjacentIndex) == oppositePlayer() &&
            chain_sets_[oppositePlayer()].getAirCountOf(adjacentIndex) == 1) {
            //将对方气为0的棋子拿走，更新拿走棋子相邻的气
            ate_piecies_indexes[i] = removeChain(oppositePlayer(), adjacentIndex);
            air_set.set(adjacentIndex);
        } else if (getPointState(adjacentPosition) == EMPTY_POINT) {
            air_set.set(adjacentIndex);
        }
    }

    Board<BOARD_LEN>::setPoint(playPosition, current_player_);
    setSpecifiedAirForAdjacentChains(playIndex, false);
    //合并刚下的棋子,并更新气
    chain_sets_[current_player_].addPiece(playIndex, air_set);


    if (chain_sets_[current_player_].getAirCountOf(playIndex) == 0) {
        //自杀棋子，拿走自杀的棋子，更新相邻棋子的气
        suicided_pieces_indexes = removeChain(current_player_, playIndex);
        if (suicided_pieces_indexes.size() == 1)
            eye_states_array_[oppositePlayer()].setEye(playIndex, true);
    } else {
        //修改下子相邻的眼的状态,以及相邻眼的斜角位置眼的状态。
        for (int i = 0; i < 4; ++i) {
            Position adjacent_position = GetAdjacentPosition(playPosition, i);
            if (!POSITION_IS_IN_BOARD(adjacent_position)) {
                continue;
            }
            if (getPointState(adjacent_position) != EMPTY_POINT) {
                continue;
            }

            PositionIndex adjacent_index = POSITION_TO_INDEX(adjacent_position);
            modifyEyesStateAndObliqueRealEyesState(adjacent_index);
        }

        // 修改下子相邻位置，是否为真的眼
        for (int i = 0; i < 4; ++i) {
            Position adjacent_position = GetAdjacentPosition(playPosition, i);
            if (!POSITION_IS_IN_BOARD(adjacent_position)) {
                continue;
            }
            if (getPointState(adjacent_position) != EMPTY_POINT) {
                continue;
            }

            PositionIndex adjacent_indx = POSITION_TO_INDEX(adjacent_position);
            modifyRealEyesState(current_player_, adjacent_indx);
        }

        // 修改下子斜角位置，是否为真的眼
        for (int i = 0; i < 4; ++i) {
            Position oblq_pos = GetObliquePosition(playPosition, i);
            if (!POSITION_IS_IN_BOARD(oblq_pos)) {
                continue;
            }
            if (getPointState(oblq_pos) != EMPTY_POINT) {
                continue;
            }

            PositionIndex oblq_indx = POSITION_TO_INDEX(oblq_pos);
            modifyRealEyesState(current_player_, oblq_indx);
            modifyRealEyesState(oppositePlayer(), oblq_indx);
        }


        ko_indx_ = -1;
        PositionIndex single_ate_piece_index = -1;

        for (int i = 0; i < 4; ++i) {
            const auto &ate_piece_indexes = ate_piecies_indexes[i];
            if (ate_piece_indexes.size() == 1) {
                single_ate_piece_index = ate_piece_indexes[0];
            }
        }

        if (single_ate_piece_index != -1 && getPointState(playIndex) == current_player_ &&
            chain_sets_[current_player_].getAirCountOf(playIndex) == 1 &&
            chain_sets_[current_player_].getPieces(playIndex).size() == 1) {
            ko_indx_ = single_ate_piece_index; //表示这是个劫
        }
    }

    modifyRealEyesPlayableState();
    current_player_ = oppositePlayer();
}

template <BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::modifyRealEyesPlayableState() {
    auto &calculator = PstionAndIndxCtrl<BOARD_LEN>::getInstance();

    for (int i = 0; i < 2; ++i) {
        const auto &eye_states = eye_states_array_.at(i);
        std::vector<PositionIndex> real_eyes = eye_states.getRealEyes();

        for (PositionIndex eye_index : real_eyes) {
            const Position &eye_position = calculator.getPosition(eye_index);

            for (int j = 0; j < 4; ++j) {
                Position adjacent_position = GetAdjacentPosition(eye_position, j);
                if (calculator.isInBoard(adjacent_position)) {
                    PositionIndex adjacent_index = calculator.getIndex(adjacent_position);
                    PlayerType force = static_cast<PlayerType>(i);
                    PlayerType opposite_force = oppositePlayer(force);
                    if (chain_sets_[i].getAirCountOf(adjacent_index) == 1) {
                        playerable_index_.at(opposite_force).set(eye_index);
                    } else {
                        playerable_index_.at(opposite_force).reset(eye_index);
                    }
                    break;
                }
            }
        }
    }
}

template <BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::modifyEyesStateAndObliqueRealEyesState(PositionIndex indx) {
    assert(getPointState(indx) == EMPTY_POINT);
    const auto &calculator = PstionAndIndxCtrl<BOARD_LEN>::getInstance();
    const Position &position = calculator.getPosition(indx);

    for (int i = 0; i < 4; ++i) {
        const Position adjacent_position = GetAdjacentPosition(position, i);
        if (!calculator.isInBoard(adjacent_position)) {
            continue;
        } else if (getPointState(adjacent_position) != current_player_) {
            eye_states_array_[current_player_].setEye(indx, false); //与我相邻的位置是空，但这个空的位置的相邻位置有对方的棋子或空的，则不算我的眼
            return;
        }
    }

    eye_states_array_[current_player_].setEye(indx, true); //与我相邻的位置是空，且这个空的位置的相邻位置都是我的棋子，所以这个算我的眼

    for (int i = 0; i < 4; ++i) {
        const Position oblq_position = GetObliquePosition(position, i);
        if (!calculator.isInBoard(oblq_position)) {
            continue;
        }

        PositionIndex oblq_indx = calculator.getIndex(oblq_position);
        if (getPointState(oblq_indx) == EMPTY_POINT) { //斜角的位置是空，判断空的这个位置是否为真的眼。
            modifyRealEyesState(current_player_, oblq_indx);
        }
    }
}

template <BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::modifyRealEyesState(PlayerType force, PositionIndex indx) {
    assert(getPointState(indx) == EMPTY_POINT);

    bool is_eye = eye_states_array_[force].isEye(indx);

    if (!is_eye) {
        eye_states_array_[force].setRealEye(indx, false); //眼都不是，肯定不会是真的眼
        return;
    }

    auto &calculator = PstionAndIndxCtrl<BOARD_LEN>::getInstance();
    const Position &position = calculator.getPosition(indx);
    PositionIndex piece_or_eye_count = 0; //斜角中眼和自己棋子的总个数

    for (int i = 0; i < 4; ++i) {
        Position oblq_pos = GetObliquePosition(position, i);
        if (!calculator.isInBoard(oblq_pos)) {
            continue;
        }

        PositionIndex oblq_indx = calculator.getIndex(oblq_pos);

        if (isSelfPieceOrEye(force, oblq_indx)) {
            ++piece_or_eye_count;
        }
    }

    static const PositionIndex TABLE[3] = {3, 2, 1}; //如果眼是在中心，则piece_or_eye_count要3个才算是真的眼。是边缘需要2，角落需要1
    auto state = calculator.centralOrEdgeOrCorner(position);
    if (TABLE[state] <= piece_or_eye_count) {
        setRealEyeAsTrue(force, indx);
    } else {
        playerable_index_[force].set(indx);
        playerable_index_[oppositePlayer(force)].set(indx);
    }
}

template <BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::setRealEyeAsTrue(PlayerType force, PositionIndex indx) {
    eye_states_array_[force].setRealEye(indx, true);
    //在眼的地方只有自己能下子，其它玩家不能下子
    playerable_index_.at(force).set(indx);
    playerable_index_.at(oppositePlayer(force)).reset(indx); 
}

template <BoardLen BOARD_LEN>
bool FullBoard<BOARD_LEN>::isEnd() {
    return playerable_index_[BLACK_PLAYER].none() &&
        playerable_index_[WHITE_PLAYER].none();
}

template <BoardLen BOARD_LEN>
bool FullBoard<BOARD_LEN>::isSelfPieceOrEye(PlayerType force, PositionIndex indx) const {
    return getPointState(indx) == force || eye_states_array_[force].isEye(indx);
}

template <BoardLen BOARD_LEN>
std::vector<PositionIndex> FullBoard<BOARD_LEN>::playableIndexes(PlayerType type) const {
    PositionIndex c = playerable_index_[type].count();
    std::vector<PositionIndex> v(c);

    PositionIndex vi = 0;
    for (BoardLen i = 0; i < playerable_index_[type].size(); ++i) {
        if (playerable_index_[type][i]) {
            v[vi++] = i;
        }
    }
    return v;
}

template <BoardLen BOARD_LEN>
vector<PositionIndex> FullBoard<BOARD_LEN>::removeChain(PlayerType force, PositionIndex indx) {
    auto &chain_set_ptr = chain_sets_[force];
    auto chain_set_pieces = chain_set_ptr.getPieces(indx);
    chain_sets_[force].removeListByPiece(indx);

    for (PositionIndex ind : chain_set_pieces) {
        Board<BOARD_LEN>::setPoint(ind, EMPTY_POINT);
        for (int i = 0; i < 2; ++i) {
            playerable_index_[i].set(ind);
        }
    }

    for (PositionIndex indx : chain_set_pieces) {
        setSpecifiedAirForAdjacentChains(indx, true);
    }

    return chain_set_pieces;
}

template<BoardLen BOARD_LEN>
void FullBoard<BOARD_LEN>::setSpecifiedAirForAdjacentChains(PositionIndex indx,
                                                            bool v) {
  auto &ins = PstionAndIndxCtrl<BOARD_LEN>::getInstance();
  const Position &pos = ins.getPosition(indx);

  for (int i = 0; i < 4; ++i) {
    Position adj_pos = GetAdjacentPosition(pos, i);
    if (!ins.isInBoard(adj_pos)) {
      continue;
    }

    PositionIndex adj_i = ins.getIndex(adj_pos);
    PointState pnt = getPointState(adj_i);
    if (pnt == EMPTY_POINT) {
      continue;
    }

    chain_sets_[pnt].setAir(adj_i, indx, v);
  }
}
} // namespace board
} // namespace foolgo

#endif