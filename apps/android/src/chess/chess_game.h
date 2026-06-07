#ifndef CAT_CHESS_GAME_MODEL_H
#define CAT_CHESS_GAME_MODEL_H

#include "chess_board.h"

typedef struct {
    ChessBoard board;
} ChessGame;

void chess_game_init(ChessGame* game);

#endif
