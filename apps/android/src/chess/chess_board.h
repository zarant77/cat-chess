#ifndef CAT_CHESS_BOARD_H
#define CAT_CHESS_BOARD_H

#define CHESS_BOARD_SIZE 8
#define CHESS_BOARD_SQUARE_COUNT 64

typedef enum {
    CHESS_PIECE_NONE = 0,
    CHESS_PIECE_PAWN,
    CHESS_PIECE_KNIGHT,
    CHESS_PIECE_BISHOP,
    CHESS_PIECE_ROOK,
    CHESS_PIECE_QUEEN,
    CHESS_PIECE_KING
} ChessPieceType;

typedef enum {
    CHESS_COLOR_WHITE = 0,
    CHESS_COLOR_BLACK = 1
} ChessColor;

typedef struct {
    ChessPieceType type;
    ChessColor color;
} ChessPiece;

typedef struct {
    ChessPiece squares[CHESS_BOARD_SQUARE_COUNT];
    ChessColor sideToMove;
} ChessBoard;

void chess_board_init(ChessBoard* board);
ChessPiece chess_board_get_piece(const ChessBoard* board, int square);
void chess_board_set_piece(ChessBoard* board, int square, ChessPiece piece);
int chess_board_square(int file, int rank);
int chess_board_move_piece(ChessBoard* board, int from_square, int to_square);

#endif
