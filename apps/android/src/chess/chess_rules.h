#ifndef CAT_CHESS_RULES_H
#define CAT_CHESS_RULES_H

#include "chess_board.h"
#include "chess_move.h"

int chess_rules_is_pseudo_legal(const ChessBoard* board, ChessMove move);

#endif
