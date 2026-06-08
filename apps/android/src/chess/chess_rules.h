#ifndef CAT_CHESS_RULES_H
#define CAT_CHESS_RULES_H

#include <stdbool.h>

#include "chess_board.h"
#include "chess_move.h"

typedef struct {
    ChessMove moves[256];
    int count;
} ChessMoveList;

typedef enum {
    CHESS_GAME_ONGOING,
    CHESS_GAME_CHECKMATE,
    CHESS_GAME_STALEMATE
} ChessGameStatus;

void chess_generate_legal_moves(const ChessBoard* board, ChessMoveList* out_moves);
bool chess_is_move_legal(const ChessBoard* board, const ChessMove* move);
bool chess_apply_move(ChessBoard* board, const ChessMove* move);
bool chess_is_square_attacked(const ChessBoard* board, int square, ChessColor by_color);
bool chess_is_in_check(const ChessBoard* board, ChessColor color);
ChessGameStatus chess_get_game_status(const ChessBoard* board);

#endif
