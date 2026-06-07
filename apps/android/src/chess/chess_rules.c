#include "chess_rules.h"

int chess_rules_is_pseudo_legal(const ChessBoard* board, ChessMove move) {
    ChessPiece moving;
    ChessPiece target;

    if (board == 0
            || move.fromSquare < 0
            || move.fromSquare >= CHESS_BOARD_SQUARE_COUNT
            || move.toSquare < 0
            || move.toSquare >= CHESS_BOARD_SQUARE_COUNT) {
        return 0;
    }

    moving = chess_board_get_piece(board, move.fromSquare);
    target = chess_board_get_piece(board, move.toSquare);

    return moving.type != CHESS_PIECE_NONE
            && moving.color == board->sideToMove
            && (target.type == CHESS_PIECE_NONE || target.color != moving.color);
}
