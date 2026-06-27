#ifndef CAT_CHESS_MOVE_LOG_H
#define CAT_CHESS_MOVE_LOG_H

#include "chess_rules.h"

#define CHESS_MOVE_LOG_MAX 256
#define CHESS_MOVE_LOG_UCI_MAX 8
#define CHESS_MOVE_LOG_DISPLAY_MAX 16

typedef struct {
    int move_number;
    ChessColor color;
    int from;
    int to;
    char uci[CHESS_MOVE_LOG_UCI_MAX];
    char display[CHESS_MOVE_LOG_DISPLAY_MAX];
    ChessPieceType captured_piece;
    ChessColor captured_color;
    ChessPieceType promotion;
} ChessMoveLogEntry;

ChessPiece chess_move_log_captured_piece(const ChessBoard* board, const ChessMove* move);
void chess_move_log_entry_from_move(
        const ChessBoard* before,
        const ChessMove* move,
        const ChessBoard* after,
        ChessMoveLogEntry* out_entry
);

#endif
