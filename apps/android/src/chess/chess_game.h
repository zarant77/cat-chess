#ifndef CAT_CHESS_GAME_MODEL_H
#define CAT_CHESS_GAME_MODEL_H

#include "chess_board.h"
#include "chess_rules.h"

typedef struct {
    ChessBoard board;
} ChessGame;

void chess_game_init(ChessGame* game);
void chess_game_get_legal_moves_for_square(const ChessGame* game, int square, ChessMoveList* out_moves);

#endif
