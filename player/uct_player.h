#ifndef FOOLGO_SRC_PLAYER_UCT_PLAYER_H_

#define FOOLGO_SRC_PLAYER_UCT_PLAYER_H_

#include <atomic>
#include <boost/lexical_cast.hpp>
#include <log4cplus/logger.h>
#include <cassert>
#include <condition_variable>
#include <cmath>
#include <cstdint>
#include <future>
#include <mutex>
#include <memory>
#include <thread>

#include "../board/force.h"
#include "../board/full_board.h"
#include "../board/position.h"
#include "../game/monte_carlo_game.h"
#include "node_record.h"
#include "passable_player.h"
#include "transposition_table.h"

namespace foolgo {
namespace player {

template<board::BoardLen BOARD_LEN>
class UctPlayer : public PassablePlayer<BOARD_LEN> {
 public:
  UctPlayer(uint32_t seed, int mc_game_count_per_move, int thread_count);

 protected:
  board::PositionIndex NextMoveWithPlayableBoard(
      const board::FullBoard<BOARD_LEN> &full_board);

 private:
  int mc_game_count_per_move_;
  TranspositionTable<BOARD_LEN> transposition_table_;
  uint32_t seed_;
  int thread_count_;
  mutable std::mutex mutex_;

  static log4cplus::Logger logger_;

  void SearchAndModifyNodes(const board::FullBoard<BOARD_LEN> &full_board,
                            std::atomic<int> *mc_game_count_ptr,
                            std::atomic<bool> *is_end_ptr,
                            int thread_index);
  board::PositionIndex MaxUcbChild(
      const board::FullBoard<BOARD_LEN> &full_board,
      int thread_index);
  float ModifyAverageProfitAndReturnNewProfit(
      board::FullBoard<BOARD_LEN> *full_board_ptr,
      std::atomic<int> *mc_game_count_ptr,
      int thread_index);
  board::PositionIndex BestChild(const board::FullBoard<BOARD_LEN> &full_board);
  void LogProfits(const board::FullBoard<BOARD_LEN> &full_board);
};

template<board::BoardLen BOARD_LEN>
log4cplus::Logger UctPlayer<BOARD_LEN>::logger_ =
    log4cplus::Logger::getInstance("foolgo.player.UctPlayer");

namespace {

float Ucb(const NodeRecord &node_record, int visited_count_sum) {
  assert(node_record.GetVisitedTime() > 0);
  return node_record.GetAverageProfit()
      + sqrt(2 * log(visited_count_sum) / node_record.GetVisitedTime());
}

template<board::BoardLen BOARD_LEN>
float GetRegionRatio(const board::FullBoard<BOARD_LEN> &full_board,
                board::Force force) {
  int black_region = full_board.BlackRegion();
  float black_ratio = static_cast<float>(black_region)
      / board::BoardLenSquare<BOARD_LEN>();
  return force == board::Force::BLACK_FORCE ? black_ratio : 1 - black_ratio;
}

}

template<board::BoardLen BOARD_LEN>
UctPlayer<BOARD_LEN>::UctPlayer(uint32_t seed, int mc_game_count_per_move,
                                int thread_count)
    : seed_(seed),
      mc_game_count_per_move_(mc_game_count_per_move),
      thread_count_(thread_count) {}

template<board::BoardLen BOARD_LEN>
board::PositionIndex UctPlayer<BOARD_LEN>::NextMoveWithPlayableBoard(
      const board::FullBoard<BOARD_LEN> &full_board) {
  std::atomic<int> current_mc_game_count(0);
  std::atomic<bool> is_end(false);
  std::vector<std::future<void>> futures;

//  SearchAndModifyNodes(full_board, &current_mc_game_count, &is_end);

  for (int i=0; i<thread_count_; ++i) {
    auto f = std::async(std::launch::async,
                        &UctPlayer<BOARD_LEN>::SearchAndModifyNodes, this,
                        std::ref(full_board),
                        &current_mc_game_count,
                        &is_end,
                        i);
    futures.push_back(std::move(f));
  }

  for (std::future<void> &future : futures) {
    LOG4CPLUS_DEBUG(logger_, "waiting...");
    future.wait();
  }

  LOG4CPLUS_DEBUG(logger_, "wait end");

  LogProfits(full_board);

  LOG4CPLUS_DEBUG(logger_, "log profits end");

  return BestChild(full_board);
}

template<board::BoardLen BOARD_LEN>
void UctPlayer<BOARD_LEN>::SearchAndModifyNodes(
    const board::FullBoard<BOARD_LEN> &full_board,
    std::atomic<int> *mc_game_count_ptr,
    std::atomic<bool> *is_end_ptr,
    int thread_index) {
    while (*mc_game_count_ptr < mc_game_count_per_move_) {
    board::PositionIndex max_ucb_index = MaxUcbChild(full_board, thread_index);
    board::FullBoard<BOARD_LEN> max_ucb_child;
    max_ucb_child.Copy(full_board);
    board::Play(&max_ucb_child, max_ucb_index);
    ModifyAverageProfitAndReturnNewProfit(&max_ucb_child, mc_game_count_ptr,
                                          thread_index);
  }

  LOG4CPLUS_DEBUG(logger_, "should notify");
}

template<board::BoardLen BOARD_LEN>
board::PositionIndex UctPlayer<BOARD_LEN>::MaxUcbChild(
    const board::FullBoard<BOARD_LEN> &full_board,
    int thread_index) {
  std::lock_guard<std::mutex> lock(mutex_);

  board::Force current_force = board::NextForce(full_board);
  auto playable_index_vector = full_board.PlayableIndexes(current_force);

  assert(!playable_index_vector.empty());

  int visited_count_sum = 0;
  std::vector<board::PositionIndex> null_indexes;

  for (board::PositionIndex position_index : playable_index_vector) {
    const NodeRecord *node_record_ptr = transposition_table_.GetChild(
        full_board, position_index);
    if (node_record_ptr == nullptr) {
      null_indexes.push_back(position_index); //可以下子，但没有模拟下子的位置
    } else if (null_indexes.empty()) {
      visited_count_sum += node_record_ptr->GetVisitedTime();
    }
  }

    //所有可以下子的位置，都已经下过，并且至少经过了一次蒙特卡罗棋局
  if (!null_indexes.empty()) {
    int index = thread_index % thread_count_;
    return null_indexes.at(index < null_indexes.size() ? index : 0); //没有下子，则在这个位置下子
    }

  float max_ucb = -1.0f;
  board::PositionIndex max_ucb_index = board::POSITION_INDEX_PASS;

  for (board::PositionIndex position_index : playable_index_vector) {//取胜算最大的棋子，继续在那一步模拟下子
    const NodeRecord *node_record_ptr = transposition_table_.GetChild(
        full_board, position_index);
    if (node_record_ptr == nullptr || node_record_ptr->IsInSearch()) {
      continue;
    }
     // It is guaranteed by the above loop that node_record_ptr is not nullptr.
    float ucb = Ucb(*node_record_ptr, visited_count_sum);
    if (ucb > max_ucb
        && !full_board.IsSuicide(
            board::Move(board::NextForce(full_board), position_index))) {
      max_ucb = ucb;
      max_ucb_index = position_index;
    }
  }

  return max_ucb_index;
}

template<board::BoardLen BOARD_LEN>
float UctPlayer<BOARD_LEN>::ModifyAverageProfitAndReturnNewProfit(
    board::FullBoard<BOARD_LEN> *full_board_ptr,
    std::atomic<int> *mc_game_count_ptr,
    int thread_index) {
  LOG4CPLUS_DEBUG(logger_, "full_board" << *full_board_ptr << " thread_index:"
                  << thread_index);
  float new_profit;
  NodeRecord *node_record_ptr = transposition_table_.Get(*full_board_ptr);

  if (node_record_ptr == nullptr) {
    game::MonteCarloGame<BOARD_LEN> monte_carlo_game(*full_board_ptr, seed_);
    if (!full_board_ptr->IsEnd()) { //在下子后，随机下子，直到整个游戏结束(蒙特卡罗对局)
      monte_carlo_game.Run();
    }
    ++(*mc_game_count_ptr); //进行了一次蒙特卡罗对局
    board::Force force = full_board_ptr->LastForce();
    new_profit = GetRegionRatio(monte_carlo_game.GetFullBoard(), force); //计算胜率
    player::NodeRecord node_record(1, new_profit, false);
    transposition_table_.Insert(*full_board_ptr, node_record);
  } else {
    mutex_.lock();
    node_record_ptr->SetIsInSearch(true);
    mutex_.unlock();

    if (full_board_ptr->IsEnd()) {
      ++(*mc_game_count_ptr);
      new_profit = node_record_ptr->GetAverageProfit();
    } else {
        //模拟对方下子
      if (full_board_ptr->PlayableIndexes(board::NextForce(*full_board_ptr))
          .empty()) {
        full_board_ptr->Pass(board::NextForce(*full_board_ptr)); //对方没有棋子可以下的，pass掉
      } else {
        LOG4CPLUS_DEBUG(logger_, "full_board" << *full_board_ptr <<
                        " thread_index:" << thread_index);
        board::PositionIndex max_ucb_index = MaxUcbChild(*full_board_ptr,
                                                         thread_index); //取对方最有利的下子位置
        board::Play(full_board_ptr, max_ucb_index);
      }
      LOG4CPLUS_DEBUG(
          logger_,
          "full_board" << *full_board_ptr << " thread_index:" << thread_index);
          getchar();
      new_profit = 1.0f
          - ModifyAverageProfitAndReturnNewProfit(full_board_ptr,
                                                  mc_game_count_ptr,
                                                  thread_index); //对方下子之后的胜率
      float previous_profit = node_record_ptr->GetAverageProfit();
      float modified_profit = (previous_profit
          * node_record_ptr->GetVisitedTime() + new_profit)
          / (node_record_ptr->GetVisitedTime() + 1);
      node_record_ptr->SetAverageProfit(modified_profit);
    }

    node_record_ptr->SetVisitedTimes(node_record_ptr->GetVisitedTime() + 1);
    std::lock_guard<std::mutex> lock(mutex_);
    node_record_ptr->SetIsInSearch(false);
  }
  return new_profit;
}

template<board::BoardLen BOARD_LEN>
board::PositionIndex UctPlayer<BOARD_LEN>::BestChild(
    const board::FullBoard<BOARD_LEN> &full_board) {
  board::Force force = board::NextForce(full_board);
  auto playable_index_vector = full_board.PlayableIndexes(force);
  int max_visited_count = -1;
  board::PositionIndex most_visited_index;

  for (board::PositionIndex index : playable_index_vector) {
    const NodeRecord *node_record = transposition_table_.GetChild(full_board,
                                                                  index);
    assert(node_record != nullptr);
    if (node_record->GetVisitedTime() > max_visited_count) {
      max_visited_count = node_record->GetVisitedTime();
      most_visited_index = index;
    }
  }

  return most_visited_index;
}

template<board::BoardLen BOARD_LEN>
void UctPlayer<BOARD_LEN>::LogProfits(
    const board::FullBoard<BOARD_LEN> &full_board) {
  auto get_profit_str =
      [this, &full_board](board::PositionIndex position_index) {
    board::Force force = board::NextForce(full_board);
    auto indexes = full_board.PlayableIndexes(force);
    auto it = std::find(indexes.begin(), indexes.end(), position_index);
    if (it == indexes.end()) {
      return 'N' + std::string(3, ' ');
    } else {
      NodeRecord *node_record = transposition_table_.GetChild(full_board,
                                                              position_index);
      assert(node_record != nullptr);
      float profit = node_record->GetAverageProfit();
      return boost::lexical_cast<std::string>(profit).substr(0, 4);
    }
  };

  LOG4CPLUS_DEBUG(logger_,
                 "profits:"<< board::ToString<BOARD_LEN>(get_profit_str, 4));
}

}
}

#endif /* FOOLGO_SRC_PLAYER_UCT_PLAYER_H_ */
