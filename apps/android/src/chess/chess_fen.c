#include "chess_fen.h"

static ChessPieceType chess_fen_piece_type(char value) {
    if (value == 'p' || value == 'P') {
        return CHESS_PIECE_PAWN;
    }
    if (value == 'n' || value == 'N') {
        return CHESS_PIECE_KNIGHT;
    }
    if (value == 'b' || value == 'B') {
        return CHESS_PIECE_BISHOP;
    }
    if (value == 'r' || value == 'R') {
        return CHESS_PIECE_ROOK;
    }
    if (value == 'q' || value == 'Q') {
        return CHESS_PIECE_QUEEN;
    }
    if (value == 'k' || value == 'K') {
        return CHESS_PIECE_KING;
    }

    return CHESS_PIECE_NONE;
}

static char chess_fen_piece_char(ChessPiece piece) {
    char value = '1';

    if (piece.type == CHESS_PIECE_PAWN) {
        value = 'p';
    } else if (piece.type == CHESS_PIECE_KNIGHT) {
        value = 'n';
    } else if (piece.type == CHESS_PIECE_BISHOP) {
        value = 'b';
    } else if (piece.type == CHESS_PIECE_ROOK) {
        value = 'r';
    } else if (piece.type == CHESS_PIECE_QUEEN) {
        value = 'q';
    } else if (piece.type == CHESS_PIECE_KING) {
        value = 'k';
    }

    if (piece.color == CHESS_COLOR_WHITE && value >= 'a' && value <= 'z') {
        value = (char)(value - 'a' + 'A');
    }

    return value;
}

static int chess_fen_append_char(char* buffer, int buffer_size, int offset, char value) {
    if (offset < 0 || offset + 1 >= buffer_size) {
        return -1;
    }
    buffer[offset] = value;
    buffer[offset + 1] = '\0';
    return offset + 1;
}

static int chess_fen_append_int(char* buffer, int buffer_size, int offset, int value) {
    char digits[12];
    int count = 0;

    if (value == 0) {
        return chess_fen_append_char(buffer, buffer_size, offset, '0');
    }
    if (value < 0) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, '-');
        if (offset < 0) {
            return -1;
        }
        value = -value;
    }
    while (value > 0 && count < (int)sizeof(digits)) {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
    }
    while (count > 0) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, digits[--count]);
        if (offset < 0) {
            return -1;
        }
    }

    return offset;
}

static const char* chess_fen_parse_int(const char* text, int* out_value) {
    int value = 0;

    if (text == 0 || out_value == 0 || *text < '0' || *text > '9') {
        return 0;
    }

    while (*text >= '0' && *text <= '9') {
        value = value * 10 + (*text - '0');
        ++text;
    }

    *out_value = value;
    return text;
}

bool chess_board_from_fen(ChessBoard* board, const char* fen) {
    int file = 0;
    int rank = 7;

    if (board == 0 || fen == 0) {
        return false;
    }

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        board->squares[square] = chess_piece_empty();
    }

    while (*fen != '\0' && *fen != ' ') {
        if (*fen == '/') {
            if (file != 8 || rank <= 0) {
                return false;
            }
            file = 0;
            rank -= 1;
        } else if (*fen >= '1' && *fen <= '8') {
            file += *fen - '0';
            if (file > 8) {
                return false;
            }
        } else {
            ChessPieceType type = chess_fen_piece_type(*fen);
            ChessColor color = (*fen >= 'A' && *fen <= 'Z') ? CHESS_COLOR_WHITE : CHESS_COLOR_BLACK;
            int square = chess_board_square(file, rank);
            if (type == CHESS_PIECE_NONE || square < 0 || file >= 8) {
                return false;
            }
            board->squares[square] = chess_piece_make(type, color);
            file += 1;
        }
        ++fen;
    }
    if (file != 8 || rank != 0 || *fen != ' ') {
        return false;
    }
    ++fen;

    if (*fen == 'w') {
        board->side_to_move = CHESS_COLOR_WHITE;
    } else if (*fen == 'b') {
        board->side_to_move = CHESS_COLOR_BLACK;
    } else {
        return false;
    }
    ++fen;
    if (*fen != ' ') {
        return false;
    }
    ++fen;

    board->white_can_castle_kingside = false;
    board->white_can_castle_queenside = false;
    board->black_can_castle_kingside = false;
    board->black_can_castle_queenside = false;
    if (*fen == '-') {
        ++fen;
    } else {
        while (*fen != '\0' && *fen != ' ') {
            if (*fen == 'K') {
                board->white_can_castle_kingside = true;
            } else if (*fen == 'Q') {
                board->white_can_castle_queenside = true;
            } else if (*fen == 'k') {
                board->black_can_castle_kingside = true;
            } else if (*fen == 'q') {
                board->black_can_castle_queenside = true;
            } else {
                return false;
            }
            ++fen;
        }
    }
    if (*fen != ' ') {
        return false;
    }
    ++fen;

    board->en_passant_square = -1;
    if (*fen == '-') {
        ++fen;
    } else {
        if (fen[0] < 'a' || fen[0] > 'h' || fen[1] < '1' || fen[1] > '8') {
            return false;
        }
        board->en_passant_square = chess_board_square(fen[0] - 'a', fen[1] - '1');
        fen += 2;
    }
    if (*fen != ' ') {
        return false;
    }
    ++fen;

    fen = chess_fen_parse_int(fen, &board->halfmove_clock);
    if (fen == 0 || *fen != ' ') {
        return false;
    }
    ++fen;
    fen = chess_fen_parse_int(fen, &board->fullmove_number);
    return fen != 0;
}

bool chess_board_to_fen(const ChessBoard* board, char* buffer, int buffer_size) {
    int offset = 0;
    bool has_castling = false;

    if (board == 0 || buffer == 0 || buffer_size <= 0) {
        return false;
    }
    buffer[0] = '\0';

    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;
        for (int file = 0; file < CHESS_BOARD_SIZE; ++file) {
            ChessPiece piece = chess_board_get_piece(board, chess_board_square(file, rank));
            if (piece.type == CHESS_PIECE_NONE) {
                empty_count += 1;
                continue;
            }
            if (empty_count > 0) {
                offset = chess_fen_append_char(buffer, buffer_size, offset, (char)('0' + empty_count));
                empty_count = 0;
            }
            offset = chess_fen_append_char(buffer, buffer_size, offset, chess_fen_piece_char(piece));
            if (offset < 0) {
                return false;
            }
        }
        if (empty_count > 0) {
            offset = chess_fen_append_char(buffer, buffer_size, offset, (char)('0' + empty_count));
        }
        if (rank > 0) {
            offset = chess_fen_append_char(buffer, buffer_size, offset, '/');
        }
        if (offset < 0) {
            return false;
        }
    }

    offset = chess_fen_append_char(buffer, buffer_size, offset, ' ');
    offset = chess_fen_append_char(buffer, buffer_size, offset, board->side_to_move == CHESS_COLOR_WHITE ? 'w' : 'b');
    offset = chess_fen_append_char(buffer, buffer_size, offset, ' ');
    if (offset < 0) {
        return false;
    }

    if (board->white_can_castle_kingside) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, 'K');
        has_castling = true;
    }
    if (board->white_can_castle_queenside) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, 'Q');
        has_castling = true;
    }
    if (board->black_can_castle_kingside) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, 'k');
        has_castling = true;
    }
    if (board->black_can_castle_queenside) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, 'q');
        has_castling = true;
    }
    if (!has_castling) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, '-');
    }
    offset = chess_fen_append_char(buffer, buffer_size, offset, ' ');

    if (board->en_passant_square >= 0) {
        offset = chess_fen_append_char(buffer, buffer_size, offset, (char)('a' + chess_board_file(board->en_passant_square)));
        offset = chess_fen_append_char(buffer, buffer_size, offset, (char)('1' + chess_board_rank(board->en_passant_square)));
    } else {
        offset = chess_fen_append_char(buffer, buffer_size, offset, '-');
    }
    offset = chess_fen_append_char(buffer, buffer_size, offset, ' ');
    offset = chess_fen_append_int(buffer, buffer_size, offset, board->halfmove_clock);
    offset = chess_fen_append_char(buffer, buffer_size, offset, ' ');
    offset = chess_fen_append_int(buffer, buffer_size, offset, board->fullmove_number);

    return offset >= 0;
}
