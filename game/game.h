#include <array>
#include <memory>
using std::array;

#include "../board/full_board.h"
#include "../player/human_player.h"

using foolgo::board::FullBoard;
using foolgo::board::MAIN_BOARD_LEN;
using foolgo::board::Position;
using foolgo::board::PositionIndex;

using foolgo::player::HumanPlayer;
using foolgo::player::Player;

using foolgo::player::BLACK_PLAYER;
using foolgo::player::NO_PLAYER;
using foolgo::player::PlayerType;
using foolgo::player::WHITE_PLAYER;

namespace foolgo {
namespace game {

class Game {
  public:
    Game(std::unique_ptr<Player<MAIN_BOARD_LEN>> black_player, std::unique_ptr<Player<MAIN_BOARD_LEN>> white_player) {
        players[BLACK_PLAYER] = std::move(black_player);
        players[WHITE_PLAYER] = std::move(white_player);
        std::cout << board << std::endl;
    }

    PlayerType run();
    bool isEnd() {
        return board.isEnd();
    }

  private:
    FullBoard<MAIN_BOARD_LEN> board;
    array<std::unique_ptr<Player<MAIN_BOARD_LEN>>, 2> players;
};

PlayerType Game::run() {
    PlayerType movePlayer;
    PositionIndex playIndex;
    while (!isEnd()) {
        movePlayer = board.getMovePlayer();
        while(1)
        {
            try {
                playIndex = players[movePlayer]->playMove(board);
                board.playMove(playIndex);
                break;
            } catch (const std::out_of_range &err) {
                std::cout << err.what() << std::flush;
                continue;
            }
        }
        std::cout << board << std::endl;
    }
    return NO_PLAYER;
}
} // namespace game
} // namespace foolgo