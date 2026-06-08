#ifndef CAT_CHESS_AI_H
#define CAT_CHESS_AI_H

#include <stdbool.h>

#include "chess_board.h"
#include "chess_move.h"

typedef enum {
    CHESS_AI_EASY,
    CHESS_AI_NORMAL
} ChessAiDifficulty;

bool chess_ai_choose_move(const ChessBoard* board, ChessAiDifficulty difficulty, ChessMove* out_move);
int chess_ai_pick_move(const ChessBoard* board, ChessMove* out_move);

#endif
