#include "chess_rules.h"

static int abs_int(int value) {
    return value < 0 ? -value : value;
}

static bool chess_square_valid(int square) {
    return square >= 0 && square < CHESS_BOARD_SQUARE_COUNT;
}

static bool chess_piece_same_color(ChessPiece left, ChessPiece right) {
    return left.type != CHESS_PIECE_NONE && right.type != CHESS_PIECE_NONE && left.color == right.color;
}

static void chess_move_list_add(ChessMoveList* list, int from, int to, ChessPieceType promotion) {
    if (list == 0 || list->count >= (int)(sizeof(list->moves) / sizeof(list->moves[0]))) {
        return;
    }

    list->moves[list->count].from = from;
    list->moves[list->count].to = to;
    list->moves[list->count].promotion = promotion;
    list->count += 1;
}

static void chess_move_list_add_pawn_move(ChessMoveList* list, int from, int to, ChessColor color) {
    int promotion_rank = color == CHESS_COLOR_WHITE ? 7 : 0;

    if (chess_board_rank(to) == promotion_rank) {
        chess_move_list_add(list, from, to, CHESS_PIECE_QUEEN);
        chess_move_list_add(list, from, to, CHESS_PIECE_ROOK);
        chess_move_list_add(list, from, to, CHESS_PIECE_BISHOP);
        chess_move_list_add(list, from, to, CHESS_PIECE_KNIGHT);
        return;
    }

    chess_move_list_add(list, from, to, CHESS_PIECE_NONE);
}

static bool chess_path_clear(const ChessBoard* board, int from, int to, int file_step, int rank_step) {
    int file = chess_board_file(from) + file_step;
    int rank = chess_board_rank(from) + rank_step;
    int to_file = chess_board_file(to);
    int to_rank = chess_board_rank(to);

    while (file != to_file || rank != to_rank) {
        int square = chess_board_square(file, rank);
        if (!chess_square_valid(square) || chess_board_get_piece(board, square).type != CHESS_PIECE_NONE) {
            return false;
        }
        file += file_step;
        rank += rank_step;
    }

    return true;
}

static bool chess_can_piece_attack_square(const ChessBoard* board, int from, int square) {
    ChessPiece piece = chess_board_get_piece(board, from);
    int from_file = chess_board_file(from);
    int from_rank = chess_board_rank(from);
    int to_file = chess_board_file(square);
    int to_rank = chess_board_rank(square);
    int file_delta = to_file - from_file;
    int rank_delta = to_rank - from_rank;

    if (piece.type == CHESS_PIECE_NONE || !chess_square_valid(square)) {
        return false;
    }

    if (piece.type == CHESS_PIECE_PAWN) {
        int direction = piece.color == CHESS_COLOR_WHITE ? 1 : -1;
        return rank_delta == direction && abs_int(file_delta) == 1;
    }
    if (piece.type == CHESS_PIECE_KNIGHT) {
        return (abs_int(file_delta) == 1 && abs_int(rank_delta) == 2)
                || (abs_int(file_delta) == 2 && abs_int(rank_delta) == 1);
    }
    if (piece.type == CHESS_PIECE_BISHOP) {
        if (abs_int(file_delta) != abs_int(rank_delta) || file_delta == 0) {
            return false;
        }
        return chess_path_clear(board, from, square, file_delta > 0 ? 1 : -1, rank_delta > 0 ? 1 : -1);
    }
    if (piece.type == CHESS_PIECE_ROOK) {
        if ((file_delta != 0 && rank_delta != 0) || (file_delta == 0 && rank_delta == 0)) {
            return false;
        }
        return chess_path_clear(
                board,
                from,
                square,
                file_delta == 0 ? 0 : (file_delta > 0 ? 1 : -1),
                rank_delta == 0 ? 0 : (rank_delta > 0 ? 1 : -1)
        );
    }
    if (piece.type == CHESS_PIECE_QUEEN) {
        if (abs_int(file_delta) == abs_int(rank_delta) && file_delta != 0) {
            return chess_path_clear(board, from, square, file_delta > 0 ? 1 : -1, rank_delta > 0 ? 1 : -1);
        }
        if ((file_delta == 0 || rank_delta == 0) && (file_delta != 0 || rank_delta != 0)) {
            return chess_path_clear(
                    board,
                    from,
                    square,
                    file_delta == 0 ? 0 : (file_delta > 0 ? 1 : -1),
                    rank_delta == 0 ? 0 : (rank_delta > 0 ? 1 : -1)
            );
        }
        return false;
    }
    if (piece.type == CHESS_PIECE_KING) {
        return abs_int(file_delta) <= 1 && abs_int(rank_delta) <= 1
                && (file_delta != 0 || rank_delta != 0);
    }

    return false;
}

bool chess_is_square_attacked(const ChessBoard* board, int square, ChessColor by_color) {
    if (board == 0 || !chess_square_valid(square)) {
        return false;
    }

    for (int from = 0; from < CHESS_BOARD_SQUARE_COUNT; ++from) {
        ChessPiece piece = chess_board_get_piece(board, from);
        if (piece.type != CHESS_PIECE_NONE
                && piece.color == by_color
                && chess_can_piece_attack_square(board, from, square)) {
            return true;
        }
    }

    return false;
}

bool chess_is_in_check(const ChessBoard* board, ChessColor color) {
    if (board == 0) {
        return false;
    }

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type == CHESS_PIECE_KING && piece.color == color) {
            return chess_is_square_attacked(board, square, chess_color_opposite(color));
        }
    }

    return false;
}

static void chess_generate_pawn_moves(const ChessBoard* board, int from, ChessMoveList* moves) {
    ChessPiece piece = chess_board_get_piece(board, from);
    int file = chess_board_file(from);
    int rank = chess_board_rank(from);
    int direction = piece.color == CHESS_COLOR_WHITE ? 1 : -1;
    int start_rank = piece.color == CHESS_COLOR_WHITE ? 1 : 6;
    int one = chess_board_square(file, rank + direction);

    if (chess_square_valid(one) && chess_board_get_piece(board, one).type == CHESS_PIECE_NONE) {
        chess_move_list_add_pawn_move(moves, from, one, piece.color);
        if (rank == start_rank) {
            int two = chess_board_square(file, rank + direction * 2);
            if (chess_square_valid(two) && chess_board_get_piece(board, two).type == CHESS_PIECE_NONE) {
                chess_move_list_add(moves, from, two, CHESS_PIECE_NONE);
            }
        }
    }

    for (int file_delta = -1; file_delta <= 1; file_delta += 2) {
        int target = chess_board_square(file + file_delta, rank + direction);
        ChessPiece capture = chess_board_get_piece(board, target);
        if (!chess_square_valid(target)) {
            continue;
        }
        if (capture.type != CHESS_PIECE_NONE
                && capture.color != piece.color
                && capture.type != CHESS_PIECE_KING) {
            chess_move_list_add_pawn_move(moves, from, target, piece.color);
        } else if (target == board->en_passant_square) {
            chess_move_list_add(moves, from, target, CHESS_PIECE_NONE);
        }
    }
}

static void chess_generate_step_moves(
        const ChessBoard* board,
        int from,
        const int offsets[][2],
        int offset_count,
        ChessMoveList* moves
) {
    ChessPiece piece = chess_board_get_piece(board, from);
    int file = chess_board_file(from);
    int rank = chess_board_rank(from);

    for (int index = 0; index < offset_count; ++index) {
        int target = chess_board_square(file + offsets[index][0], rank + offsets[index][1]);
        ChessPiece target_piece = chess_board_get_piece(board, target);
        if (chess_square_valid(target)
                && !chess_piece_same_color(piece, target_piece)
                && target_piece.type != CHESS_PIECE_KING) {
            chess_move_list_add(moves, from, target, CHESS_PIECE_NONE);
        }
    }
}

static void chess_generate_slide_moves(
        const ChessBoard* board,
        int from,
        const int directions[][2],
        int direction_count,
        ChessMoveList* moves
) {
    ChessPiece piece = chess_board_get_piece(board, from);
    int from_file = chess_board_file(from);
    int from_rank = chess_board_rank(from);

    for (int index = 0; index < direction_count; ++index) {
        int file = from_file + directions[index][0];
        int rank = from_rank + directions[index][1];
        while (file >= 0 && file < CHESS_BOARD_SIZE && rank >= 0 && rank < CHESS_BOARD_SIZE) {
            int target = chess_board_square(file, rank);
            ChessPiece target_piece = chess_board_get_piece(board, target);
            if (target_piece.type == CHESS_PIECE_NONE) {
                chess_move_list_add(moves, from, target, CHESS_PIECE_NONE);
            } else {
                if (target_piece.color != piece.color && target_piece.type != CHESS_PIECE_KING) {
                    chess_move_list_add(moves, from, target, CHESS_PIECE_NONE);
                }
                break;
            }
            file += directions[index][0];
            rank += directions[index][1];
        }
    }
}

static void chess_generate_castle_moves(const ChessBoard* board, int from, ChessMoveList* moves) {
    ChessPiece king = chess_board_get_piece(board, from);
    ChessColor enemy = chess_color_opposite(king.color);

    if (king.color == CHESS_COLOR_WHITE && from == chess_board_square(4, 0)
            && !chess_is_in_check(board, CHESS_COLOR_WHITE)) {
        if (board->white_can_castle_kingside
                && chess_board_get_piece(board, chess_board_square(7, 0)).type == CHESS_PIECE_ROOK
                && chess_board_get_piece(board, chess_board_square(7, 0)).color == CHESS_COLOR_WHITE
                && chess_board_get_piece(board, chess_board_square(5, 0)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(6, 0)).type == CHESS_PIECE_NONE
                && !chess_is_square_attacked(board, chess_board_square(5, 0), enemy)
                && !chess_is_square_attacked(board, chess_board_square(6, 0), enemy)) {
            chess_move_list_add(moves, from, chess_board_square(6, 0), CHESS_PIECE_NONE);
        }
        if (board->white_can_castle_queenside
                && chess_board_get_piece(board, chess_board_square(0, 0)).type == CHESS_PIECE_ROOK
                && chess_board_get_piece(board, chess_board_square(0, 0)).color == CHESS_COLOR_WHITE
                && chess_board_get_piece(board, chess_board_square(1, 0)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(2, 0)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(3, 0)).type == CHESS_PIECE_NONE
                && !chess_is_square_attacked(board, chess_board_square(2, 0), enemy)
                && !chess_is_square_attacked(board, chess_board_square(3, 0), enemy)) {
            chess_move_list_add(moves, from, chess_board_square(2, 0), CHESS_PIECE_NONE);
        }
    } else if (king.color == CHESS_COLOR_BLACK && from == chess_board_square(4, 7)
            && !chess_is_in_check(board, CHESS_COLOR_BLACK)) {
        if (board->black_can_castle_kingside
                && chess_board_get_piece(board, chess_board_square(7, 7)).type == CHESS_PIECE_ROOK
                && chess_board_get_piece(board, chess_board_square(7, 7)).color == CHESS_COLOR_BLACK
                && chess_board_get_piece(board, chess_board_square(5, 7)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(6, 7)).type == CHESS_PIECE_NONE
                && !chess_is_square_attacked(board, chess_board_square(5, 7), enemy)
                && !chess_is_square_attacked(board, chess_board_square(6, 7), enemy)) {
            chess_move_list_add(moves, from, chess_board_square(6, 7), CHESS_PIECE_NONE);
        }
        if (board->black_can_castle_queenside
                && chess_board_get_piece(board, chess_board_square(0, 7)).type == CHESS_PIECE_ROOK
                && chess_board_get_piece(board, chess_board_square(0, 7)).color == CHESS_COLOR_BLACK
                && chess_board_get_piece(board, chess_board_square(1, 7)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(2, 7)).type == CHESS_PIECE_NONE
                && chess_board_get_piece(board, chess_board_square(3, 7)).type == CHESS_PIECE_NONE
                && !chess_is_square_attacked(board, chess_board_square(2, 7), enemy)
                && !chess_is_square_attacked(board, chess_board_square(3, 7), enemy)) {
            chess_move_list_add(moves, from, chess_board_square(2, 7), CHESS_PIECE_NONE);
        }
    }
}

static void chess_generate_pseudo_moves_for_square(const ChessBoard* board, int from, ChessMoveList* moves) {
    static const int knight_offsets[8][2] = {
            {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
    };
    static const int king_offsets[8][2] = {
            {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}
    };
    static const int bishop_dirs[4][2] = {
            {1, 1}, {1, -1}, {-1, -1}, {-1, 1}
    };
    static const int rook_dirs[4][2] = {
            {1, 0}, {0, -1}, {-1, 0}, {0, 1}
    };
    static const int queen_dirs[8][2] = {
            {1, 1}, {1, -1}, {-1, -1}, {-1, 1}, {1, 0}, {0, -1}, {-1, 0}, {0, 1}
    };
    ChessPiece piece = chess_board_get_piece(board, from);

    if (piece.type == CHESS_PIECE_PAWN) {
        chess_generate_pawn_moves(board, from, moves);
    } else if (piece.type == CHESS_PIECE_KNIGHT) {
        chess_generate_step_moves(board, from, knight_offsets, 8, moves);
    } else if (piece.type == CHESS_PIECE_BISHOP) {
        chess_generate_slide_moves(board, from, bishop_dirs, 4, moves);
    } else if (piece.type == CHESS_PIECE_ROOK) {
        chess_generate_slide_moves(board, from, rook_dirs, 4, moves);
    } else if (piece.type == CHESS_PIECE_QUEEN) {
        chess_generate_slide_moves(board, from, queen_dirs, 8, moves);
    } else if (piece.type == CHESS_PIECE_KING) {
        chess_generate_step_moves(board, from, king_offsets, 8, moves);
        chess_generate_castle_moves(board, from, moves);
    }
}

static void chess_clear_castling_for_square(ChessBoard* board, int square) {
    if (square == chess_board_square(0, 0)) {
        board->white_can_castle_queenside = false;
    } else if (square == chess_board_square(7, 0)) {
        board->white_can_castle_kingside = false;
    } else if (square == chess_board_square(0, 7)) {
        board->black_can_castle_queenside = false;
    } else if (square == chess_board_square(7, 7)) {
        board->black_can_castle_kingside = false;
    }
}

static void chess_apply_move_unchecked(ChessBoard* board, const ChessMove* move) {
    ChessPiece moving = chess_board_get_piece(board, move->from);
    ChessPiece captured = chess_board_get_piece(board, move->to);
    int from_rank = chess_board_rank(move->from);
    int to_rank = chess_board_rank(move->to);
    int file_delta = chess_board_file(move->to) - chess_board_file(move->from);
    bool en_passant_capture = moving.type == CHESS_PIECE_PAWN
            && move->to == board->en_passant_square
            && captured.type == CHESS_PIECE_NONE
            && file_delta != 0;

    if (moving.type == CHESS_PIECE_KING) {
        if (moving.color == CHESS_COLOR_WHITE) {
            board->white_can_castle_kingside = false;
            board->white_can_castle_queenside = false;
        } else {
            board->black_can_castle_kingside = false;
            board->black_can_castle_queenside = false;
        }
    }

    if (moving.type == CHESS_PIECE_ROOK) {
        chess_clear_castling_for_square(board, move->from);
    }
    if (captured.type == CHESS_PIECE_ROOK) {
        chess_clear_castling_for_square(board, move->to);
    }

    board->squares[move->to] = moving;
    board->squares[move->from] = chess_piece_empty();

    if (en_passant_capture) {
        int captured_square = chess_board_square(chess_board_file(move->to), from_rank);
        board->squares[captured_square] = chess_piece_empty();
        captured.type = CHESS_PIECE_PAWN;
    }

    if (moving.type == CHESS_PIECE_KING && abs_int(file_delta) == 2) {
        if (move->to == chess_board_square(6, 0)) {
            board->squares[chess_board_square(5, 0)] = board->squares[chess_board_square(7, 0)];
            board->squares[chess_board_square(7, 0)] = chess_piece_empty();
        } else if (move->to == chess_board_square(2, 0)) {
            board->squares[chess_board_square(3, 0)] = board->squares[chess_board_square(0, 0)];
            board->squares[chess_board_square(0, 0)] = chess_piece_empty();
        } else if (move->to == chess_board_square(6, 7)) {
            board->squares[chess_board_square(5, 7)] = board->squares[chess_board_square(7, 7)];
            board->squares[chess_board_square(7, 7)] = chess_piece_empty();
        } else if (move->to == chess_board_square(2, 7)) {
            board->squares[chess_board_square(3, 7)] = board->squares[chess_board_square(0, 7)];
            board->squares[chess_board_square(0, 7)] = chess_piece_empty();
        }
    }

    if (moving.type == CHESS_PIECE_PAWN && (to_rank == 0 || to_rank == 7)) {
        ChessPieceType promotion = move->promotion == CHESS_PIECE_NONE ? CHESS_PIECE_QUEEN : move->promotion;
        board->squares[move->to] = chess_piece_make(promotion, moving.color);
    }

    board->en_passant_square = -1;
    if (moving.type == CHESS_PIECE_PAWN && abs_int(to_rank - from_rank) == 2) {
        board->en_passant_square = chess_board_square(chess_board_file(move->from), (from_rank + to_rank) / 2);
    }

    if (moving.type == CHESS_PIECE_PAWN || captured.type != CHESS_PIECE_NONE) {
        board->halfmove_clock = 0;
    } else {
        board->halfmove_clock += 1;
    }

    if (board->side_to_move == CHESS_COLOR_BLACK) {
        board->fullmove_number += 1;
    }
    board->side_to_move = chess_color_opposite(board->side_to_move);
}

void chess_generate_legal_moves(const ChessBoard* board, ChessMoveList* out_moves) {
    ChessMoveList pseudo;

    if (out_moves == 0) {
        return;
    }
    out_moves->count = 0;
    if (board == 0) {
        return;
    }

    pseudo.count = 0;
    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type != CHESS_PIECE_NONE && piece.color == board->side_to_move) {
            chess_generate_pseudo_moves_for_square(board, square, &pseudo);
        }
    }

    for (int index = 0; index < pseudo.count; ++index) {
        ChessBoard copy = *board;
        ChessColor moving_color = board->side_to_move;
        chess_apply_move_unchecked(&copy, &pseudo.moves[index]);
        if (!chess_is_in_check(&copy, moving_color)) {
            chess_move_list_add(out_moves, pseudo.moves[index].from, pseudo.moves[index].to, pseudo.moves[index].promotion);
        }
    }
}

bool chess_is_move_legal(const ChessBoard* board, const ChessMove* move) {
    ChessMoveList moves;

    if (board == 0 || move == 0) {
        return false;
    }

    chess_generate_legal_moves(board, &moves);
    for (int index = 0; index < moves.count; ++index) {
        ChessMove candidate = moves.moves[index];
        if (candidate.from == move->from && candidate.to == move->to) {
            if (candidate.promotion == move->promotion
                    || (candidate.promotion == CHESS_PIECE_QUEEN && move->promotion == CHESS_PIECE_NONE)) {
                return true;
            }
        }
    }

    return false;
}

bool chess_apply_move(ChessBoard* board, const ChessMove* move) {
    ChessMove normalized;

    if (!chess_is_move_legal(board, move)) {
        return false;
    }

    normalized = *move;
    if (chess_board_get_piece(board, normalized.from).type == CHESS_PIECE_PAWN
            && (chess_board_rank(normalized.to) == 0 || chess_board_rank(normalized.to) == 7)
            && normalized.promotion == CHESS_PIECE_NONE) {
        normalized.promotion = CHESS_PIECE_QUEEN;
    }

    chess_apply_move_unchecked(board, &normalized);
    return true;
}

ChessGameStatus chess_get_game_status(const ChessBoard* board) {
    ChessMoveList moves;

    if (board == 0) {
        return CHESS_GAME_ONGOING;
    }

    chess_generate_legal_moves(board, &moves);
    if (moves.count > 0) {
        return CHESS_GAME_ONGOING;
    }

    return chess_is_in_check(board, board->side_to_move)
            ? CHESS_GAME_CHECKMATE
            : CHESS_GAME_STALEMATE;
}
