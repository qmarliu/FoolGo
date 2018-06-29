#include <iostream>
#include <memory>

#include "board/full_board.h"
#include "game/game.h"
#include "utility/math.h"
#include "player/human_player.h"
#include "player/random_player.h"

using foolgo::math::GetTimeSeed;

using foolgo::board::FullBoard;

using foolgo::game::Game;

using foolgo::player::Player;
using foolgo::player::HumanPlayer;
using foolgo::player::RandomPlayer;
using foolgo::player::NO_PLAYER;
using foolgo::player::WHITE_PLAYER;
using foolgo::player::BLACK_PLAYER;
using foolgo::player::PlayerType;

int main() {
    auto seed = GetTimeSeed();

    std::unique_ptr<Player<MAIN_BOARD_LEN>> black_player(new HumanPlayer<MAIN_BOARD_LEN>());
    std::unique_ptr<Player<MAIN_BOARD_LEN>> white_player(new RandomPlayer<MAIN_BOARD_LEN>(seed));
    Game game(std::move(black_player), std::move(white_player));

    PlayerType winPlayer = game.run();
    if (winPlayer == BLACK_PLAYER) {
        std::cout << "黑方胜" << std::endl;
    } else if (winPlayer == WHITE_PLAYER) {
        std::cout << "白方胜" << std::endl;
    } else {
        std::cout << "和棋" << std::endl;
    }

    return 0;
}