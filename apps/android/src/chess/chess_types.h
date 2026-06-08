#ifndef CAT_CHESS_TYPES_H
#define CAT_CHESS_TYPES_H

#include <stdbool.h>

#define CHESS_BOARD_SIZE 8
#define CHESS_BOARD_SQUARE_COUNT 64

typedef enum {
    CHESS_COLOR_WHITE = 0,
    CHESS_COLOR_BLACK = 1
} ChessColor;

typedef enum {
    CHESS_PIECE_NONE = 0,
    CHESS_PIECE_PAWN,
    CHESS_PIECE_KNIGHT,
    CHESS_PIECE_BISHOP,
    CHESS_PIECE_ROOK,
    CHESS_PIECE_QUEEN,
    CHESS_PIECE_KING
} ChessPieceType;

typedef struct {
    ChessPieceType type;
    ChessColor color;
} ChessPiece;

/* Square indexing is 0=a1, 1=b1, ... 7=h1, 8=a2, ... 63=h8. */
typedef struct {
    ChessPiece squares[CHESS_BOARD_SQUARE_COUNT];
    ChessColor side_to_move;

    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;

    int en_passant_square;
    int halfmove_clock;
    int fullmove_number;
} ChessBoard;

#endif
