#include "chess_move_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void chess_move_log_square_name(int square, char* buffer, int buffer_size) {
    if (buffer == 0 || buffer_size < 3 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    buffer[0] = (char)('a' + chess_board_file(square));
    buffer[1] = (char)('1' + chess_board_rank(square));
    buffer[2] = '\0';
}

static char chess_move_log_promotion_char(ChessPieceType piece) {
    if (piece == CHESS_PIECE_QUEEN) {
        return 'Q';
    }
    if (piece == CHESS_PIECE_ROOK) {
        return 'R';
    }
    if (piece == CHESS_PIECE_BISHOP) {
        return 'B';
    }
    if (piece == CHESS_PIECE_KNIGHT) {
        return 'N';
    }
    return '\0';
}

ChessPiece chess_move_log_captured_piece(const ChessBoard* board, const ChessMove* move) {
    ChessPiece moving;
    ChessPiece captured;
    int en_passant_capture_square;

    if (board == 0 || move == 0) {
        return chess_piece_empty();
    }

    moving = chess_board_get_piece(board, move->from);
    captured = chess_board_get_piece(board, move->to);
    if (captured.type != CHESS_PIECE_NONE) {
        return captured;
    }

    if (moving.type == CHESS_PIECE_PAWN
            && move->to == board->en_passant_square
            && chess_board_file(move->from) != chess_board_file(move->to)) {
        en_passant_capture_square = chess_board_square(chess_board_file(move->to), chess_board_rank(move->from));
        return chess_board_get_piece(board, en_passant_capture_square);
    }

    return chess_piece_empty();
}

void chess_move_log_entry_from_move(
        const ChessBoard* before,
        const ChessMove* move,
        const ChessBoard* after,
        ChessMoveLogEntry* out_entry
) {
    ChessPiece moving;
    ChessPiece captured;
    ChessPieceType promotion;
    char from[3] = "";
    char to[3] = "";
    char promotion_char;
    const char* suffix = "";
    int is_castle;

    if (out_entry == 0) {
        return;
    }
    memset(out_entry, 0, sizeof(*out_entry));
    out_entry->captured_piece = CHESS_PIECE_NONE;
    out_entry->captured_color = CHESS_COLOR_WHITE;
    out_entry->promotion = CHESS_PIECE_NONE;

    if (before == 0 || move == 0) {
        return;
    }

    moving = chess_board_get_piece(before, move->from);
    captured = chess_move_log_captured_piece(before, move);
    promotion = move->promotion;
    if (moving.type == CHESS_PIECE_PAWN
            && (chess_board_rank(move->to) == 0 || chess_board_rank(move->to) == 7)
            && promotion == CHESS_PIECE_NONE) {
        promotion = CHESS_PIECE_QUEEN;
    }

    out_entry->move_number = before->fullmove_number;
    out_entry->color = moving.color;
    out_entry->from = move->from;
    out_entry->to = move->to;
    out_entry->captured_piece = captured.type;
    out_entry->captured_color = captured.color;
    out_entry->promotion = promotion;
    chess_move_to_uci(move, out_entry->uci, sizeof(out_entry->uci));

    if (after != 0) {
        ChessGameStatus status = chess_get_game_status(after);
        if (status == CHESS_GAME_CHECKMATE) {
            suffix = "#";
        } else if (chess_is_in_check(after, after->side_to_move)) {
            suffix = "+";
        }
    }

    is_castle = moving.type == CHESS_PIECE_KING
            && abs(chess_board_file(move->to) - chess_board_file(move->from)) == 2;
    if (is_castle) {
        snprintf(
                out_entry->display,
                sizeof(out_entry->display),
                "%s%s",
                chess_board_file(move->to) > chess_board_file(move->from) ? "O-O" : "O-O-O",
                suffix
        );
        return;
    }

    chess_move_log_square_name(move->from, from, sizeof(from));
    chess_move_log_square_name(move->to, to, sizeof(to));
    promotion_char = chess_move_log_promotion_char(promotion);
    if (promotion_char != '\0') {
        snprintf(
                out_entry->display,
                sizeof(out_entry->display),
                "%s%c%s=%c%s",
                from,
                captured.type == CHESS_PIECE_NONE ? '-' : 'x',
                to,
                promotion_char,
                suffix
        );
        return;
    }

    snprintf(
            out_entry->display,
            sizeof(out_entry->display),
            "%s%c%s%s",
            from,
            captured.type == CHESS_PIECE_NONE ? '-' : 'x',
            to,
            suffix
    );
}
