#include "chess_ai.h"

#include "chess_rules.h"

static unsigned int chess_ai_seed = 0x00c0ffeeu;

static unsigned int chess_ai_next_random(void) {
    chess_ai_seed = chess_ai_seed * 1103515245u + 12345u;
    return (chess_ai_seed / 65536u) % 32768u;
}

static int chess_ai_piece_value(ChessPieceType type) {
    if (type == CHESS_PIECE_PAWN) {
        return 100;
    }
    if (type == CHESS_PIECE_KNIGHT) {
        return 320;
    }
    if (type == CHESS_PIECE_BISHOP) {
        return 330;
    }
    if (type == CHESS_PIECE_ROOK) {
        return 500;
    }
    if (type == CHESS_PIECE_QUEEN) {
        return 900;
    }

    return 0;
}

static int chess_ai_evaluate_material(const ChessBoard* board, ChessColor color) {
    int score = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int value = chess_ai_piece_value(piece.type);
        if (piece.type == CHESS_PIECE_NONE) {
            continue;
        }
        score += piece.color == color ? value : -value;
    }

    return score;
}

bool chess_ai_choose_move(const ChessBoard* board, ChessAiDifficulty difficulty, ChessMove* out_move) {
    ChessMoveList moves;
    ChessColor ai_color;
    int best_score = -1000000;
    int best_index = 0;

    if (board == 0 || out_move == 0) {
        return false;
    }

    chess_generate_legal_moves(board, &moves);
    if (moves.count <= 0) {
        return false;
    }

    if (difficulty == CHESS_AI_EASY) {
        *out_move = moves.moves[(int)(chess_ai_next_random() % (unsigned int)moves.count)];
        return true;
    }

    ai_color = board->side_to_move;
    for (int index = 0; index < moves.count; ++index) {
        ChessBoard copy = *board;
        ChessPiece target = chess_board_get_piece(board, moves.moves[index].to);
        int score;

        chess_apply_move(&copy, &moves.moves[index]);
        score = chess_ai_evaluate_material(&copy, ai_color);
        if (target.type != CHESS_PIECE_NONE) {
            score += chess_ai_piece_value(target.type) / 8;
        }
        if (moves.moves[index].promotion != CHESS_PIECE_NONE) {
            score += chess_ai_piece_value(moves.moves[index].promotion) / 4;
        }
        if (chess_is_in_check(&copy, chess_color_opposite(ai_color))) {
            score += 35;
        }
        score += (int)(chess_ai_next_random() % 17u) - 8;

        if (score > best_score) {
            best_score = score;
            best_index = index;
        }
    }

    *out_move = moves.moves[best_index];
    return true;
}

int chess_ai_pick_move(const ChessBoard* board, ChessMove* out_move) {
    return chess_ai_choose_move(board, CHESS_AI_NORMAL, out_move) ? 1 : 0;
}
