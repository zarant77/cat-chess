#ifndef CAT_CHESS_FEN_H
#define CAT_CHESS_FEN_H

#include <stdbool.h>

#include "chess_board.h"

bool chess_board_from_fen(ChessBoard* board, const char* fen);
bool chess_board_to_fen(const ChessBoard* board, char* buffer, int buffer_size);

#endif
