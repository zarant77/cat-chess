#include "chess_game.h"

void chess_game_init(ChessGame* game) {
    if (game == 0) {
        return;
    }

    chess_board_init(&game->board);
}
