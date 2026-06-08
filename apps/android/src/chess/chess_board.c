#include "chess_board.h"

ChessPiece chess_piece_make(ChessPieceType type, ChessColor color) {
    ChessPiece piece;
    piece.type = type;
    piece.color = color;
    return piece;
}

ChessPiece chess_piece_empty(void) {
    return chess_piece_make(CHESS_PIECE_NONE, CHESS_COLOR_WHITE);
}

int chess_board_square(int file, int rank) {
    if (file < 0 || file >= CHESS_BOARD_SIZE || rank < 0 || rank >= CHESS_BOARD_SIZE) {
        return -1;
    }

    return rank * CHESS_BOARD_SIZE + file;
}

int chess_board_file(int square) {
    if (square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return -1;
    }

    return square % CHESS_BOARD_SIZE;
}

int chess_board_rank(int square) {
    if (square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return -1;
    }

    return square / CHESS_BOARD_SIZE;
}

ChessColor chess_color_opposite(ChessColor color) {
    return color == CHESS_COLOR_WHITE ? CHESS_COLOR_BLACK : CHESS_COLOR_WHITE;
}

void chess_board_set_start_position(ChessBoard* board) {
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
        board->squares[square] = chess_piece_empty();
    }

    for (int file = 0; file < CHESS_BOARD_SIZE; ++file) {
        board->squares[chess_board_square(file, 0)] = chess_piece_make(back_rank[file], CHESS_COLOR_WHITE);
        board->squares[chess_board_square(file, 1)] = chess_piece_make(CHESS_PIECE_PAWN, CHESS_COLOR_WHITE);
        board->squares[chess_board_square(file, 6)] = chess_piece_make(CHESS_PIECE_PAWN, CHESS_COLOR_BLACK);
        board->squares[chess_board_square(file, 7)] = chess_piece_make(back_rank[file], CHESS_COLOR_BLACK);
    }

    board->side_to_move = CHESS_COLOR_WHITE;
    board->white_can_castle_kingside = true;
    board->white_can_castle_queenside = true;
    board->black_can_castle_kingside = true;
    board->black_can_castle_queenside = true;
    board->en_passant_square = -1;
    board->halfmove_clock = 0;
    board->fullmove_number = 1;
}

void chess_board_init(ChessBoard* board) {
    chess_board_set_start_position(board);
}

ChessPiece chess_board_get_piece(const ChessBoard* board, int square) {
    if (board == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return chess_piece_empty();
    }

    return board->squares[square];
}

void chess_board_set_piece(ChessBoard* board, int square, ChessPiece piece) {
    if (board == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    board->squares[square] = piece;
}
