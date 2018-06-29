#ifndef FOOLGO_CHAIN_SET_H
#define FOOLGO_CHAIN_SET_H

#include <bitset>

#include "def.h"

namespace foolgo {
namespace board {
typedef size_t AirCount;
template <BoardLen BOARD_LEN>
class ChainSet {
  public:
    void addPiece(PositionIndex playIndex, const std::bitset<BoardLenSquare<BOARD_LEN>()> &air_set);
    void createList(PositionIndex playIndex, const std::bitset<BoardLenSquare<BOARD_LEN>()> &air_set);
    PositionIndex mergeList(PositionIndex head_a, PositionIndex head_b);

    void removeListByPiece(PositionIndex piece_i);
    AirCount getAirCountOf(PositionIndex piece_i) const;
    void setAir(board::PositionIndex indx, board::PositionIndex air_i, bool v);
    std::vector<board::PositionIndex> getPieces(board::PositionIndex piece_i) const;

  private:
    std::vector<board::PositionIndex> getPiecesOfChain(board::PositionIndex list_i) const;
    void removeList(PositionIndex head);
    static const board::PositionIndex NONE_LIST = -1;
    struct List {
        PositionIndex tail_;
        PositionIndex len_;
        std::bitset<BoardLenSquare<BOARD_LEN>()> air_set_;
    } lists_[BoardLenSquare<BOARD_LEN>()];

    struct Node {
        PositionIndex next_;
        PositionIndex list_head_;
        Node() {
            list_head_ = NONE_LIST;
            next_ = -1;
        }
    } nodes_[BoardLenSquare<BOARD_LEN>()];
};

template <BoardLen BOARD_LEN>
AirCount ChainSet<BOARD_LEN>::getAirCountOf(PositionIndex piece_i) const {
    return lists_[nodes_[piece_i].list_head_].air_set_.count();
}

template <BoardLen BOARD_LEN>
void ChainSet<BOARD_LEN>::createList(PositionIndex index, const std::bitset<BoardLenSquare<BOARD_LEN>()> &air_set) {
    nodes_[index].list_head_ = index;
    List *list = lists_ + index;
    list->tail_ = index;
    list->len_ = 1;
    list->air_set_ = air_set;
}

template <BoardLen BOARD_LEN>
void ChainSet<BOARD_LEN>::addPiece(PositionIndex index, 
                const std::bitset<BoardLenSquare<BOARD_LEN>()> &air_set) {
    createList(index, air_set);

    const board::Position &pos = INDEX_TO_POSITION(index);
    board::PositionIndex list_i = index;
    for (int i = 0; i < 4; ++i) {
        board::Position adj_pos = GetAdjacentPosition(pos, i);
        if (POSITION_IS_IN_BOARD(adj_pos)) {
            board::PositionIndex adj_i = POSITION_TO_INDEX(adj_pos);
            board::PositionIndex adj_list = nodes_[adj_i].list_head_;
            if (adj_list == ChainSet<BOARD_LEN>::NONE_LIST) {
                continue;
            }
            if (adj_list == list_i) {
                continue;
            }

            list_i = mergeList(list_i, adj_list);
        }
    }
}

template <BoardLen BOARD_LEN>
PositionIndex ChainSet<BOARD_LEN>::mergeList(PositionIndex head_a, PositionIndex head_b)
{
    assert(head_a != head_b);
    if (lists_[head_a].len_ > lists_[head_b].len_)
        std::swap(head_a, head_b);

    for (int i = head_a;; i = nodes_[i].next_) {
        nodes_[i].list_head_ = head_b;
        if (i == lists_[head_a].tail_) {
            break;
        }
        assert(i != nodes_[i].next_);
    }

    List *list_b = lists_ + head_b;
    nodes_[list_b->tail_].next_ = head_a;
    list_b->tail_ = lists_[head_a].tail_;
    list_b->len_ += lists_[head_a].len_;
    list_b->air_set_ |= lists_[head_a].air_set_;
    return head_b;
}

template <board::BoardLen BOARD_LEN>
void ChainSet<BOARD_LEN>::removeListByPiece(PositionIndex piece_i) {
    board::PositionIndex list_i = nodes_[piece_i].list_head_;
    removeList(list_i);
}

template <BoardLen BOARD_LEN>
void ChainSet<BOARD_LEN>::removeList(PositionIndex head) {
    for (int i = head;; i = nodes_[i].next_) {
        nodes_[i].list_head_ = ChainSet<BOARD_LEN>::NONE_LIST;
        if (i == lists_[head].tail_)
            break;
    }
}

template<board::BoardLen BOARD_LEN>
std::vector<board::PositionIndex> ChainSet<BOARD_LEN>::getPieces(
    board::PositionIndex piece_i) const {
  return getPiecesOfChain(nodes_[piece_i].list_head_);
}

template<board::BoardLen BOARD_LEN>
std::vector<board::PositionIndex> ChainSet<BOARD_LEN>::getPiecesOfChain(
    board::PositionIndex list_i) const {
  auto pl = lists_ + list_i;
  std::vector<board::PositionIndex> v(pl->len_);
  int vi = 0;

  for (int i = list_i;; i = nodes_[i].next_) {
    v[vi++] = i;
    if (i == pl->tail_) {
      break;
    }
  }

  return v;
}

template<board::BoardLen BOARD_LEN>
void ChainSet<BOARD_LEN>::setAir(PositionIndex indx,
                                 PositionIndex air_i, bool v) {
    BitSet<BOARD_LEN> air_set;
    if (!v)
        air_set.set();
    air_set[air_i] = v;
    board::PositionIndex head = nodes_[indx].list_head_;
    assert(head != ChainSet<BOARD_LEN>::NONE_LIST);
    List *pl = lists_ + head;
    const board::BitSet<BOARD_LEN> &r_as = pl->air_set_;
    pl->air_set_ = v ? (r_as | air_set) : (r_as & air_set);
}

} // namespace board
} // namespace foolgo

#endif