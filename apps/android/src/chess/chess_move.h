#ifndef CAT_CHESS_MOVE_H
#define CAT_CHESS_MOVE_H

#include <stdbool.h>

#include "chess_types.h"

typedef struct {
    int from;
    int to;
    ChessPieceType promotion;
} ChessMove;

bool chess_move_parse_uci(const char* uci, ChessMove* out_move);
void chess_move_to_uci(const ChessMove* move, char* buffer, int buffer_size);

#endif
