#ifndef CAT_CHESS_BOARD_H
#define CAT_CHESS_BOARD_H

#include "chess_types.h"

void chess_board_init(ChessBoard* board);
void chess_board_set_start_position(ChessBoard* board);
ChessPiece chess_board_get_piece(const ChessBoard* board, int square);
void chess_board_set_piece(ChessBoard* board, int square, ChessPiece piece);
int chess_board_square(int file, int rank);
int chess_board_file(int square);
int chess_board_rank(int square);
ChessColor chess_color_opposite(ChessColor color);
ChessPiece chess_piece_make(ChessPieceType type, ChessColor color);
ChessPiece chess_piece_empty(void);

#endif
