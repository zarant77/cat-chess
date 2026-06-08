#include "chess_move.h"

#include "chess_board.h"

static int chess_move_parse_square(const char* text) {
    int file;
    int rank;

    if (text == 0 || text[0] < 'a' || text[0] > 'h' || text[1] < '1' || text[1] > '8') {
        return -1;
    }

    file = text[0] - 'a';
    rank = text[1] - '1';
    return chess_board_square(file, rank);
}

static ChessPieceType chess_move_parse_promotion(char value) {
    if (value == 'q') {
        return CHESS_PIECE_QUEEN;
    }
    if (value == 'r') {
        return CHESS_PIECE_ROOK;
    }
    if (value == 'b') {
        return CHESS_PIECE_BISHOP;
    }
    if (value == 'n') {
        return CHESS_PIECE_KNIGHT;
    }

    return CHESS_PIECE_NONE;
}

static char chess_move_promotion_char(ChessPieceType promotion) {
    if (promotion == CHESS_PIECE_QUEEN) {
        return 'q';
    }
    if (promotion == CHESS_PIECE_ROOK) {
        return 'r';
    }
    if (promotion == CHESS_PIECE_BISHOP) {
        return 'b';
    }
    if (promotion == CHESS_PIECE_KNIGHT) {
        return 'n';
    }

    return '\0';
}

bool chess_move_parse_uci(const char* uci, ChessMove* out_move) {
    int length = 0;
    ChessMove move;

    if (uci == 0 || out_move == 0) {
        return false;
    }

    while (uci[length] != '\0') {
        ++length;
    }
    if (length != 4 && length != 5) {
        return false;
    }

    move.from = chess_move_parse_square(uci);
    move.to = chess_move_parse_square(uci + 2);
    move.promotion = CHESS_PIECE_NONE;
    if (length == 5) {
        move.promotion = chess_move_parse_promotion(uci[4]);
        if (move.promotion == CHESS_PIECE_NONE) {
            return false;
        }
    }

    if (move.from < 0 || move.to < 0) {
        return false;
    }

    *out_move = move;
    return true;
}

void chess_move_to_uci(const ChessMove* move, char* buffer, int buffer_size) {
    int length = 4;
    char promotion;

    if (buffer == 0 || buffer_size <= 0) {
        return;
    }

    buffer[0] = '\0';
    if (move == 0 || move->from < 0 || move->from >= CHESS_BOARD_SQUARE_COUNT
            || move->to < 0 || move->to >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    promotion = chess_move_promotion_char(move->promotion);
    if (promotion != '\0') {
        length = 5;
    }
    if (buffer_size <= length) {
        return;
    }

    buffer[0] = (char)('a' + chess_board_file(move->from));
    buffer[1] = (char)('1' + chess_board_rank(move->from));
    buffer[2] = (char)('a' + chess_board_file(move->to));
    buffer[3] = (char)('1' + chess_board_rank(move->to));
    if (promotion != '\0') {
        buffer[4] = promotion;
    }
    buffer[length] = '\0';
}
