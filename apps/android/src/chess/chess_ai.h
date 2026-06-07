#ifndef CAT_CHESS_AI_H
#define CAT_CHESS_AI_H

#include "chess_board.h"
#include "chess_move.h"

int chess_ai_pick_move(const ChessBoard* board, ChessMove* out_move);

#endif
