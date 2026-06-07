#include "chess_board.h"

static ChessPiece chess_piece(ChessPieceType type, ChessColor color) {
    ChessPiece piece;
    piece.type = type;
    piece.color = color;
    return piece;
}

static ChessPiece chess_empty_piece(void) {
    return chess_piece(CHESS_PIECE_NONE, CHESS_COLOR_WHITE);
}

int chess_board_square(int file, int rank) {
    if (file < 0 || file >= CHESS_BOARD_SIZE || rank < 0 || rank >= CHESS_BOARD_SIZE) {
        return -1;
    }

    return rank * CHESS_BOARD_SIZE + file;
}

void chess_board_init(ChessBoard* board) {
    static const ChessPieceType back_rank[CHESS_BOARD_SIZE] = {
            CHESS_PIECE_ROOK,
            CHESS_PIECE_KNIGHT,
            CHESS_PIECE_BISHOP,
            CHESS_PIECE_QUEEN,
            CHESS_PIECE_KING,
            CHESS_PIECE_BISHOP,
            CHESS_PIECE_KNIGHT,
            CHESS_PIECE_ROOK
    };

    if (board == 0) {
        return;
    }

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        board->squares[square] = chess_empty_piece();
    }

    for (int file = 0; file < CHESS_BOARD_SIZE; ++file) {
        board->squares[chess_board_square(file, 0)] = chess_piece(back_rank[file], CHESS_COLOR_BLACK);
        board->squares[chess_board_square(file, 1)] = chess_piece(CHESS_PIECE_PAWN, CHESS_COLOR_BLACK);
        board->squares[chess_board_square(file, 6)] = chess_piece(CHESS_PIECE_PAWN, CHESS_COLOR_WHITE);
        board->squares[chess_board_square(file, 7)] = chess_piece(back_rank[file], CHESS_COLOR_WHITE);
    }

    board->sideToMove = CHESS_COLOR_WHITE;
}

ChessPiece chess_board_get_piece(const ChessBoard* board, int square) {
    if (board == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return chess_empty_piece();
    }

    return board->squares[square];
}

void chess_board_set_piece(ChessBoard* board, int square, ChessPiece piece) {
    if (board == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    board->squares[square] = piece;
}

int chess_board_move_piece(ChessBoard* board, int from_square, int to_square) {
    ChessPiece moving;
    ChessPiece target;

    if (board == 0
            || from_square < 0
            || from_square >= CHESS_BOARD_SQUARE_COUNT
            || to_square < 0
            || to_square >= CHESS_BOARD_SQUARE_COUNT) {
        return 0;
    }

    moving = board->squares[from_square];
    target = board->squares[to_square];

    if (moving.type == CHESS_PIECE_NONE || moving.color != board->sideToMove) {
        return 0;
    }

    if (target.type != CHESS_PIECE_NONE && target.color == moving.color) {
        return 0;
    }

    board->squares[to_square] = moving;
    board->squares[from_square] = chess_empty_piece();
    board->sideToMove = board->sideToMove == CHESS_COLOR_WHITE
            ? CHESS_COLOR_BLACK
            : CHESS_COLOR_WHITE;

    return 1;
}
