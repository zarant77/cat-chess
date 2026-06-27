#include "chess_ai.h"

#include "chess_rules.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

#define CHESS_AI_INF 1000000
#define CHESS_AI_MATE_SCORE 900000
#define CHESS_AI_MATE_BOUND 800000
#define CHESS_AI_MAX_SEARCH_DEPTH 16
#define CHESS_AI_MAX_QUIESCENCE_DEPTH 8
#ifndef CHESS_AI_TT_ENTRY_COUNT
#define CHESS_AI_TT_ENTRY_COUNT 65536
#endif
#ifndef CAT_CHESS_AI_DEBUG
#define CAT_CHESS_AI_DEBUG 0
#endif

typedef struct {
    uint64_t key;
    int score;
    int depth;
    uint8_t bound;
    uint8_t age;
    ChessMove best_move;
} ChessAiTtEntry;

typedef enum {
    CHESS_TT_BOUND_EXACT = 0,
    CHESS_TT_BOUND_LOWER = 1,
    CHESS_TT_BOUND_UPPER = 2
} ChessAiTtBound;

typedef struct {
    ChessAiConfig config;
    uint64_t deadline_ms;
    int stop_requested;
    uint64_t nodes;
    uint64_t quiescence_nodes;
    uint64_t tt_probes;
    uint64_t tt_hits;
    uint64_t tt_cutoffs;
    uint64_t tt_stores;
    ChessMove killer_moves[CHESS_AI_MAX_SEARCH_DEPTH][2];
    int history_scores[2][CHESS_BOARD_SQUARE_COUNT][CHESS_BOARD_SQUARE_COUNT];
    uint8_t search_age;
} ChessAiContext;

typedef struct {
    int bishop_pair;
    int mobility;
    int rook_open_file;
    int rook_semi_open_file;
    int rook_seventh_rank;
    int passed_pawn;
    int passed_pawn_progress;
    int protected_passed_pawn;
    int advanced_passed_bonus;
    int isolated_pawn;
    int doubled_pawn;
    int hanging_piece_divisor;
    int attacked_piece_bad_trade_divisor;
    int attacked_piece_defended_divisor;
    int hanging_queen;
    int hanging_rook;
    int defended_queen_attack;
    int defended_rook_attack;
    int king_shield_pawn;
    int king_open_shield_penalty;
    int king_in_check_penalty;
    int endgame_king_activity;
    int mating_edge;
    int mating_king_distance;
    int develop_knight;
    int develop_bishop;
    int castled_king;
    int early_queen_penalty;
    int center_occupy;
    int center_control;
    int mate_threat;
    int mate_allowed_penalty;
    int save_attacked_queen;
    int save_attacked_rook;
    int draw_pressure_start;
    int draw_pressure_step;
} ChessAiWeights;

static const ChessAiWeights chess_ai_weights = {
    .bishop_pair = 35,
    .mobility = 4,
    .rook_open_file = 28,
    .rook_semi_open_file = 14,
    .rook_seventh_rank = 12,
    .passed_pawn = 18,
    .passed_pawn_progress = 7,
    .protected_passed_pawn = 14,
    .advanced_passed_bonus = 36,
    .isolated_pawn = 12,
    .doubled_pawn = 14,
    .hanging_piece_divisor = 8,
    .attacked_piece_bad_trade_divisor = 18,
    .attacked_piece_defended_divisor = 35,
    .hanging_queen = 90,
    .hanging_rook = 45,
    .defended_queen_attack = 35,
    .defended_rook_attack = 18,
    .king_shield_pawn = 10,
    .king_open_shield_penalty = 10,
    .king_in_check_penalty = 55,
    .endgame_king_activity = 8,
    .mating_edge = 35,
    .mating_king_distance = 6,
    .develop_knight = 18,
    .develop_bishop = 14,
    .castled_king = 28,
    .early_queen_penalty = 42,
    .center_occupy = 16,
    .center_control = 7,
    .mate_threat = 45000,
    .mate_allowed_penalty = 180000,
    .save_attacked_queen = 28000,
    .save_attacked_rook = 14000,
    .draw_pressure_start = 80,
    .draw_pressure_step = 6
};

static uint32_t chess_ai_seed = 0x00c0ffeeu;
static ChessAiTtEntry chess_ai_tt[CHESS_AI_TT_ENTRY_COUNT];
static uint8_t chess_ai_tt_age = 1;
static uint64_t chess_ai_zobrist_piece[2][7][CHESS_BOARD_SQUARE_COUNT];
static uint64_t chess_ai_zobrist_side;
static uint64_t chess_ai_zobrist_castling[4];
static uint64_t chess_ai_zobrist_en_passant_file[8];
static int chess_ai_zobrist_initialized = 0;

static const int chess_ai_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};

static const int chess_ai_knight_table[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

static const int chess_ai_bishop_table[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

static const int chess_ai_rook_table[64] = {
      0,   0,   5,  10,  10,   5,   0,   0,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      5,  10,  10,  10,  10,  10,  10,   5,
      0,   0,   0,   5,   5,   0,   0,   0
};

static const int chess_ai_queen_table[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -10,   5,   5,   5,   5,   5,   0, -10,
      0,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
    -10,   0,   5,   5,   5,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

static const int chess_ai_king_mid_table[64] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

static const int chess_ai_king_end_table[64] = {
    -50, -30, -30, -30, -30, -30, -30, -50,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -50, -40, -30, -20, -20, -30, -40, -50
};

static uint32_t chess_ai_next_random(uint32_t* seed) {
    *seed = *seed * 1103515245u + 12345u;
    return (*seed / 65536u) % 32768u;
}

static uint64_t chess_ai_now_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000ull + (uint64_t)now.tv_nsec / 1000000ull;
}

static uint64_t chess_ai_splitmix64(uint64_t* state) {
    uint64_t value;

    *state += 0x9e3779b97f4a7c15ull;
    value = *state;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ull;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebull;
    return value ^ (value >> 31);
}

static void chess_ai_init_zobrist(void) {
    uint64_t seed = 0x43a7c9e3779b97f4ull;

    if (chess_ai_zobrist_initialized) {
        return;
    }

    for (int color = 0; color < 2; ++color) {
        for (int type = 0; type < 7; ++type) {
            for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
                chess_ai_zobrist_piece[color][type][square] = chess_ai_splitmix64(&seed);
            }
        }
    }
    chess_ai_zobrist_side = chess_ai_splitmix64(&seed);
    for (int index = 0; index < 4; ++index) {
        chess_ai_zobrist_castling[index] = chess_ai_splitmix64(&seed);
    }
    for (int file = 0; file < 8; ++file) {
        chess_ai_zobrist_en_passant_file[file] = chess_ai_splitmix64(&seed);
    }
    chess_ai_zobrist_initialized = 1;
}

uint64_t chess_board_compute_zobrist_hash(const ChessBoard* board) {
    uint64_t key = 0;

    if (board == 0) {
        return 0;
    }

    chess_ai_init_zobrist();
    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type != CHESS_PIECE_NONE) {
            key ^= chess_ai_zobrist_piece[piece.color][piece.type][square];
        }
    }
    if (board->side_to_move == CHESS_COLOR_BLACK) {
        key ^= chess_ai_zobrist_side;
    }
    if (board->white_can_castle_kingside) {
        key ^= chess_ai_zobrist_castling[0];
    }
    if (board->white_can_castle_queenside) {
        key ^= chess_ai_zobrist_castling[1];
    }
    if (board->black_can_castle_kingside) {
        key ^= chess_ai_zobrist_castling[2];
    }
    if (board->black_can_castle_queenside) {
        key ^= chess_ai_zobrist_castling[3];
    }
    if (board->en_passant_square >= 0 && board->en_passant_square < CHESS_BOARD_SQUARE_COUNT) {
        key ^= chess_ai_zobrist_en_passant_file[chess_board_file(board->en_passant_square)];
    }

    return key;
}

static int chess_ai_abs(int value) {
    return value < 0 ? -value : value;
}

static int chess_ai_min_int(int left, int right) {
    return left < right ? left : right;
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
    if (type == CHESS_PIECE_KING) {
        return 20000;
    }

    return 0;
}

static int chess_ai_table_square(int square, ChessColor color) {
    int file = chess_board_file(square);
    int rank = chess_board_rank(square);
    if (color == CHESS_COLOR_BLACK) {
        rank = 7 - rank;
    }
    return chess_board_square(file, rank);
}

static int chess_ai_piece_square_value(ChessPieceType type, int square, ChessColor color, bool endgame) {
    int table_square = chess_ai_table_square(square, color);

    if (type == CHESS_PIECE_PAWN) {
        return chess_ai_pawn_table[table_square];
    }
    if (type == CHESS_PIECE_KNIGHT) {
        return chess_ai_knight_table[table_square];
    }
    if (type == CHESS_PIECE_BISHOP) {
        return chess_ai_bishop_table[table_square];
    }
    if (type == CHESS_PIECE_ROOK) {
        return chess_ai_rook_table[table_square];
    }
    if (type == CHESS_PIECE_QUEEN) {
        return chess_ai_queen_table[table_square];
    }
    if (type == CHESS_PIECE_KING) {
        return endgame ? chess_ai_king_end_table[table_square] : chess_ai_king_mid_table[table_square];
    }

    return 0;
}

static bool chess_ai_move_is_castle(const ChessBoard* board, const ChessMove* move) {
    ChessPiece moving = chess_board_get_piece(board, move->from);
    return moving.type == CHESS_PIECE_KING
            && chess_ai_abs(chess_board_file(move->to) - chess_board_file(move->from)) == 2;
}

static ChessPiece chess_ai_captured_piece(const ChessBoard* board, const ChessMove* move) {
    ChessPiece moving = chess_board_get_piece(board, move->from);
    ChessPiece captured = chess_board_get_piece(board, move->to);

    if (captured.type == CHESS_PIECE_NONE
            && moving.type == CHESS_PIECE_PAWN
            && move->to == board->en_passant_square
            && chess_board_file(move->from) != chess_board_file(move->to)) {
        return chess_piece_make(CHESS_PIECE_PAWN, chess_color_opposite(moving.color));
    }

    return captured;
}

static bool chess_ai_move_gives_checkmate(const ChessBoard* board, const ChessMove* move) {
    ChessBoard copy = *board;
    ChessMoveList replies;

    if (!chess_apply_move(&copy, move)) {
        return false;
    }
    if (!chess_is_in_check(&copy, copy.side_to_move)) {
        return false;
    }

    chess_generate_legal_moves(&copy, &replies);
    return replies.count == 0;
}

static bool chess_ai_move_equal(const ChessMove* left, const ChessMove* right) {
    if (left == 0 || right == 0) {
        return false;
    }

    return left->from == right->from
            && left->to == right->to
            && left->promotion == right->promotion;
}

static bool chess_ai_move_valid(ChessMove move) {
    return move.from >= 0 && move.from < CHESS_BOARD_SQUARE_COUNT
            && move.to >= 0 && move.to < CHESS_BOARD_SQUARE_COUNT;
}

static bool chess_ai_move_is_quiet(const ChessBoard* board, const ChessMove* move) {
    return chess_ai_captured_piece(board, move).type == CHESS_PIECE_NONE
            && move->promotion == CHESS_PIECE_NONE;
}

static bool chess_ai_move_gives_check(const ChessBoard* board, const ChessMove* move) {
    ChessBoard copy;

    if (board == 0 || move == 0) {
        return false;
    }

    copy = *board;
    return chess_apply_move(&copy, move) && chess_is_in_check(&copy, copy.side_to_move);
}

static int chess_ai_smallest_attacker_value(const ChessBoard* board, int square, ChessColor color) {
    ChessBoard copy;
    ChessMoveList moves;
    int best = CHESS_AI_INF;

    if (board == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return 0;
    }

    copy = *board;
    copy.side_to_move = color;
    chess_generate_legal_moves(&copy, &moves);
    for (int index = 0; index < moves.count; ++index) {
        if (moves.moves[index].to == square) {
            ChessPiece attacker = chess_board_get_piece(board, moves.moves[index].from);
            int value = chess_ai_piece_value(attacker.type);
            if (value > 0 && value < best) {
                best = value;
            }
        }
    }

    return best == CHESS_AI_INF ? 0 : best;
}

static int chess_ai_static_exchange_eval(const ChessBoard* board, const ChessMove* move) {
    ChessBoard copy;
    ChessPiece moving;
    ChessPiece captured;
    ChessPiece landed;
    int score;
    int recapture_value;

    if (board == 0 || move == 0) {
        return 0;
    }

    moving = chess_board_get_piece(board, move->from);
    captured = chess_ai_captured_piece(board, move);
    if (captured.type == CHESS_PIECE_NONE && move->promotion == CHESS_PIECE_NONE) {
        return 0;
    }

    score = chess_ai_piece_value(captured.type);
    if (move->promotion != CHESS_PIECE_NONE) {
        score += chess_ai_piece_value(move->promotion) - chess_ai_piece_value(CHESS_PIECE_PAWN);
    }

    copy = *board;
    if (!chess_apply_move(&copy, move)) {
        return -CHESS_AI_INF;
    }

    landed = chess_board_get_piece(&copy, move->to);
    recapture_value = chess_ai_smallest_attacker_value(&copy, move->to, copy.side_to_move);
    if (recapture_value > 0 && landed.type != CHESS_PIECE_NONE) {
        score -= chess_ai_piece_value(landed.type);
    }

    (void)moving;
    return score;
}

static bool chess_ai_has_mate_in_one(const ChessBoard* board, ChessColor color) {
    ChessBoard copy;
    ChessMoveList moves;

    if (board == 0) {
        return false;
    }

    copy = *board;
    copy.side_to_move = color;
    chess_generate_legal_moves(&copy, &moves);
    for (int index = 0; index < moves.count; ++index) {
        if (chess_ai_move_gives_checkmate(&copy, &moves.moves[index])) {
            return true;
        }
    }

    return false;
}

static bool chess_ai_find_mate_in_one(const ChessBoard* board, ChessColor color, ChessMove* out_move) {
    ChessBoard copy;
    ChessMoveList moves;

    if (board == 0) {
        return false;
    }

    copy = *board;
    copy.side_to_move = color;
    chess_generate_legal_moves(&copy, &moves);
    for (int index = 0; index < moves.count; ++index) {
        if (chess_ai_move_gives_checkmate(&copy, &moves.moves[index])) {
            if (out_move != 0) {
                *out_move = moves.moves[index];
            }
            return true;
        }
    }

    return false;
}

static bool chess_ai_move_allows_mate_in_one(const ChessBoard* board, const ChessMove* move) {
    ChessBoard copy;

    if (board == 0 || move == 0) {
        return false;
    }

    copy = *board;
    if (!chess_apply_move(&copy, move)) {
        return false;
    }

    return chess_ai_has_mate_in_one(&copy, copy.side_to_move);
}

static bool chess_ai_move_creates_mate_threat(const ChessBoard* board, const ChessMove* move) {
    ChessBoard copy;
    ChessColor mover;

    if (board == 0 || move == 0) {
        return false;
    }

    mover = board->side_to_move;
    copy = *board;
    if (!chess_apply_move(&copy, move)) {
        return false;
    }
    copy.side_to_move = mover;
    return chess_ai_has_mate_in_one(&copy, mover);
}

static bool chess_ai_move_is_near_promotion(const ChessBoard* board, const ChessMove* move) {
    ChessPiece moving;
    int rank;

    if (board == 0 || move == 0) {
        return false;
    }

    moving = chess_board_get_piece(board, move->from);
    if (moving.type != CHESS_PIECE_PAWN) {
        return false;
    }

    rank = chess_board_rank(move->to);
    return (moving.color == CHESS_COLOR_WHITE && rank >= 5)
            || (moving.color == CHESS_COLOR_BLACK && rank <= 2);
}

static int chess_ai_promotion_order_value(ChessPieceType type) {
    if (type == CHESS_PIECE_QUEEN) {
        return 5000;
    }
    if (type == CHESS_PIECE_ROOK) {
        return 3000;
    }
    if (type == CHESS_PIECE_BISHOP) {
        return 2200;
    }
    if (type == CHESS_PIECE_KNIGHT) {
        return 2100;
    }

    return 0;
}

static int chess_ai_move_order_score(
        ChessAiContext* context,
        const ChessBoard* board,
        const ChessMove* move,
        int ply,
        ChessMove tt_move
) {
    ChessPiece moving = chess_board_get_piece(board, move->from);
    ChessPiece captured = chess_ai_captured_piece(board, move);
    ChessBoard copy;
    int score = 0;

    if (chess_ai_move_valid(tt_move) && chess_ai_move_equal(move, &tt_move)) {
        return 2000000;
    }
    if (chess_ai_move_gives_checkmate(board, move)) {
        score += 100000;
    }
    if (move->promotion != CHESS_PIECE_NONE) {
        score += 90000 + chess_ai_promotion_order_value(move->promotion);
    }
    if (captured.type != CHESS_PIECE_NONE) {
        int capture_delta = chess_ai_piece_value(captured.type) - chess_ai_piece_value(moving.type);
        int see = chess_ai_static_exchange_eval(board, move);
        score += 70000 + chess_ai_piece_value(captured.type) * 16 - chess_ai_piece_value(moving.type);
        if (see >= 0) {
            score += 30000 + see * 4;
        } else {
            score += see * 6;
        }
        if (capture_delta >= 0) {
            score += 12000 + capture_delta;
        }
    }

    copy = *board;
    if (chess_apply_move(&copy, move) && chess_is_in_check(&copy, copy.side_to_move)) {
        score += 10000;
    }

    if (chess_ai_move_is_castle(board, move)) {
        score += 800;
    }
    if (context != 0 && context->config.level == CHESS_AI_HARD && ply <= 1) {
        if (chess_ai_move_allows_mate_in_one(board, move)) {
            score -= chess_ai_weights.mate_allowed_penalty;
        }
        if (chess_ai_move_creates_mate_threat(board, move)) {
            score += chess_ai_weights.mate_threat;
        }
        if ((moving.type == CHESS_PIECE_QUEEN || moving.type == CHESS_PIECE_ROOK)
                && chess_is_square_attacked(board, move->from, chess_color_opposite(moving.color))) {
            ChessBoard copy = *board;
            if (chess_apply_move(&copy, move)
                    && !chess_is_square_attacked(&copy, move->to, copy.side_to_move)) {
                score += moving.type == CHESS_PIECE_QUEEN ? chess_ai_weights.save_attacked_queen : chess_ai_weights.save_attacked_rook;
            }
        }
    }
    if ((moving.type == CHESS_PIECE_KNIGHT || moving.type == CHESS_PIECE_BISHOP)
            && (chess_board_rank(move->from) == 0 || chess_board_rank(move->from) == 7)) {
        score += 120;
    }
    if (context != 0 && chess_ai_move_is_quiet(board, move)) {
        int bounded_ply = ply < CHESS_AI_MAX_SEARCH_DEPTH ? ply : CHESS_AI_MAX_SEARCH_DEPTH - 1;
        if (chess_ai_move_equal(move, &context->killer_moves[bounded_ply][0])) {
            score += 60000;
        } else if (chess_ai_move_equal(move, &context->killer_moves[bounded_ply][1])) {
            score += 55000;
        }
        score += context->history_scores[moving.color][move->from][move->to];
    }

    score += 32 - chess_ai_abs(3 - chess_board_file(move->to)) - chess_ai_abs(3 - chess_board_rank(move->to));
    return score;
}

static int chess_ai_score_move(
        ChessAiContext* context,
        const ChessBoard* board,
        const ChessMove* move,
        int ply
) {
    ChessMove empty;
    empty.from = -1;
    empty.to = -1;
    empty.promotion = CHESS_PIECE_NONE;
    return chess_ai_move_order_score(context, board, move, ply, empty);
}

static void chess_ai_order_moves(
        ChessAiContext* context,
        const ChessBoard* board,
        ChessMoveList* moves,
        int ply,
        ChessMove tt_move
) {
    int scores[256];

    for (int index = 0; index < moves->count; ++index) {
        scores[index] = chess_ai_move_valid(tt_move)
                ? chess_ai_move_order_score(context, board, &moves->moves[index], ply, tt_move)
                : chess_ai_score_move(context, board, &moves->moves[index], ply);
    }

    for (int index = 1; index < moves->count; ++index) {
        ChessMove move = moves->moves[index];
        int score = scores[index];
        int scan = index - 1;
        while (scan >= 0 && scores[scan] < score) {
            moves->moves[scan + 1] = moves->moves[scan];
            scores[scan + 1] = scores[scan];
            --scan;
        }
        moves->moves[scan + 1] = move;
        scores[scan + 1] = score;
    }
}

static int chess_ai_count_legal_moves_for(const ChessBoard* board, ChessColor color) {
    ChessBoard copy = *board;
    ChessMoveList moves;

    copy.side_to_move = color;
    chess_generate_legal_moves(&copy, &moves);
    return moves.count;
}

static bool chess_ai_is_endgame(const ChessBoard* board) {
    int queens = 0;
    int minor_major = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type == CHESS_PIECE_QUEEN) {
            queens += 1;
        } else if (piece.type == CHESS_PIECE_ROOK
                || piece.type == CHESS_PIECE_BISHOP
                || piece.type == CHESS_PIECE_KNIGHT) {
            minor_major += 1;
        }
    }

    return queens == 0 || minor_major <= 4;
}

static int chess_ai_piece_phase(ChessPieceType type) {
    if (type == CHESS_PIECE_KNIGHT || type == CHESS_PIECE_BISHOP) {
        return 1;
    }
    if (type == CHESS_PIECE_ROOK) {
        return 2;
    }
    if (type == CHESS_PIECE_QUEEN) {
        return 4;
    }
    return 0;
}

static int chess_ai_game_phase(const ChessBoard* board) {
    int phase = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        phase += chess_ai_piece_phase(piece.type);
    }
    if (phase > 24) {
        phase = 24;
    }
    return phase;
}

static int chess_ai_find_king_square(const ChessBoard* board, ChessColor color) {
    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type == CHESS_PIECE_KING && piece.color == color) {
            return square;
        }
    }
    return -1;
}

static int chess_ai_nonking_material(const ChessBoard* board, ChessColor color) {
    int material = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type != CHESS_PIECE_NONE && piece.type != CHESS_PIECE_KING && piece.color == color) {
            material += chess_ai_piece_value(piece.type);
        }
    }

    return material;
}

static int chess_ai_king_distance_score(int first_square, int second_square) {
    int file_distance;
    int rank_distance;

    if (first_square < 0 || second_square < 0) {
        return 0;
    }

    file_distance = chess_ai_abs(chess_board_file(first_square) - chess_board_file(second_square));
    rank_distance = chess_ai_abs(chess_board_rank(first_square) - chess_board_rank(second_square));
    return 14 - (file_distance + rank_distance) * 2;
}

static int chess_ai_evaluate_mating_net(const ChessBoard* board, ChessColor color) {
    ChessColor enemy = chess_color_opposite(color);
    int material = chess_ai_nonking_material(board, color);
    int enemy_material = chess_ai_nonking_material(board, enemy);
    int own_king = chess_ai_find_king_square(board, color);
    int enemy_king = chess_ai_find_king_square(board, enemy);
    int enemy_file;
    int enemy_rank;
    int edge_distance;
    int score = 0;

    if (material < enemy_material + 500 || material < 900 || enemy_king < 0) {
        return 0;
    }

    enemy_file = chess_board_file(enemy_king);
    enemy_rank = chess_board_rank(enemy_king);
    edge_distance = chess_ai_min_int(
            chess_ai_min_int(enemy_file, 7 - enemy_file),
            chess_ai_min_int(enemy_rank, 7 - enemy_rank)
    );
    score += (3 - edge_distance) * chess_ai_weights.mating_edge;
    score += chess_ai_king_distance_score(own_king, enemy_king) * chess_ai_weights.mating_king_distance;

    return score;
}

static int chess_ai_evaluate_pawns(const ChessBoard* board, ChessColor color) {
    int files[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int score = 0;
    int direction = color == CHESS_COLOR_WHITE ? 1 : -1;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type == CHESS_PIECE_PAWN && piece.color == color) {
            files[chess_board_file(square)] += 1;
        }
    }

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int file = chess_board_file(square);
        int rank = chess_board_rank(square);
        bool has_neighbor = false;
        bool passed = true;

        if (piece.type != CHESS_PIECE_PAWN || piece.color != color) {
            continue;
        }

        if (files[file] > 1) {
            score -= chess_ai_weights.doubled_pawn * (files[file] - 1);
        }
        if (file > 0 && files[file - 1] > 0) {
            has_neighbor = true;
        }
        if (file < 7 && files[file + 1] > 0) {
            has_neighbor = true;
        }
        if (!has_neighbor) {
            score -= chess_ai_weights.isolated_pawn;
        }

        for (int scan_file = file - 1; scan_file <= file + 1; ++scan_file) {
            int scan_rank = rank + direction;
            if (scan_file < 0 || scan_file > 7) {
                continue;
            }
            while (scan_rank >= 0 && scan_rank < 8) {
                ChessPiece other = chess_board_get_piece(board, chess_board_square(scan_file, scan_rank));
                if (other.type == CHESS_PIECE_PAWN && other.color != color) {
                    passed = false;
                    break;
                }
                scan_rank += direction;
            }
        }
        if (passed) {
            int progress = color == CHESS_COLOR_WHITE ? rank : 7 - rank;
            score += chess_ai_weights.passed_pawn + progress * chess_ai_weights.passed_pawn_progress;
            if (chess_is_square_attacked(board, square, color)) {
                score += chess_ai_weights.protected_passed_pawn + progress * 3;
            }
            if (progress >= 5) {
                score += chess_ai_weights.advanced_passed_bonus + progress * 8;
            }
        }
    }

    return score;
}

static int chess_ai_evaluate_rooks(const ChessBoard* board, ChessColor color) {
    int score = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int file;
        int own_pawns = 0;
        int enemy_pawns = 0;

        if (piece.type != CHESS_PIECE_ROOK || piece.color != color) {
            continue;
        }

        file = chess_board_file(square);
        for (int rank = 0; rank < 8; ++rank) {
            ChessPiece scan = chess_board_get_piece(board, chess_board_square(file, rank));
            if (scan.type == CHESS_PIECE_PAWN) {
                if (scan.color == color) {
                    own_pawns += 1;
                } else {
                    enemy_pawns += 1;
                }
            }
        }

        if (own_pawns == 0 && enemy_pawns == 0) {
            score += chess_ai_weights.rook_open_file;
        } else if (own_pawns == 0) {
            score += chess_ai_weights.rook_semi_open_file;
        }
        if (chess_board_rank(square) == (color == CHESS_COLOR_WHITE ? 6 : 1)) {
            score += chess_ai_weights.rook_seventh_rank;
        }
    }

    return score;
}

static int chess_ai_evaluate_major_piece_safety(const ChessBoard* board, ChessColor color) {
    ChessColor enemy = chess_color_opposite(color);
    int score = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int value;
        int defender;

        if ((piece.type != CHESS_PIECE_QUEEN && piece.type != CHESS_PIECE_ROOK) || piece.color != color) {
            continue;
        }
        if (!chess_is_square_attacked(board, square, enemy)) {
            continue;
        }

        value = chess_ai_piece_value(piece.type);
        defender = chess_is_square_attacked(board, square, color) ? 1 : 0;
        if (!defender) {
            score -= piece.type == CHESS_PIECE_QUEEN ? chess_ai_weights.hanging_queen : chess_ai_weights.hanging_rook;
        } else {
            score -= piece.type == CHESS_PIECE_QUEEN ? chess_ai_weights.defended_queen_attack : chess_ai_weights.defended_rook_attack;
        }
    }

    return score;
}

static int chess_ai_evaluate_piece_safety(const ChessBoard* board, ChessColor color) {
    ChessColor enemy = chess_color_opposite(color);
    int score = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int value;
        int defended;
        int attacker_value;

        if (piece.type == CHESS_PIECE_NONE || piece.type == CHESS_PIECE_KING || piece.color != color) {
            continue;
        }
        if (!chess_is_square_attacked(board, square, enemy)) {
            continue;
        }

        value = chess_ai_piece_value(piece.type);
        defended = chess_is_square_attacked(board, square, color);
        attacker_value = chess_ai_smallest_attacker_value(board, square, enemy);
        if (!defended) {
            score -= value / chess_ai_weights.hanging_piece_divisor;
        } else if (attacker_value > 0 && attacker_value < value) {
            score -= value / chess_ai_weights.attacked_piece_bad_trade_divisor;
        } else {
            score -= value / chess_ai_weights.attacked_piece_defended_divisor;
        }
    }

    return score;
}

static int chess_ai_evaluate_king_safety(const ChessBoard* board, ChessColor color, int phase) {
    int king_square = chess_ai_find_king_square(board, color);
    int score = 0;
    int file;
    int rank;
    int shield_rank;
    ChessColor enemy = chess_color_opposite(color);

    if (king_square < 0) {
        return -500;
    }

    file = chess_board_file(king_square);
    rank = chess_board_rank(king_square);
    shield_rank = rank + (color == CHESS_COLOR_WHITE ? 1 : -1);

    if (phase > 8) {
        for (int df = -1; df <= 1; ++df) {
            int shield_file = file + df;
            if (shield_file < 0 || shield_file > 7 || shield_rank < 0 || shield_rank > 7) {
                score -= 12;
                continue;
            }
            ChessPiece shield = chess_board_get_piece(board, chess_board_square(shield_file, shield_rank));
            if (shield.type == CHESS_PIECE_PAWN && shield.color == color) {
                score += chess_ai_weights.king_shield_pawn;
            } else {
                score -= chess_ai_weights.king_open_shield_penalty;
            }
        }
        if (chess_is_square_attacked(board, king_square, enemy)) {
            score -= chess_ai_weights.king_in_check_penalty;
        }
    } else {
        int center_distance = chess_ai_abs(file - 3) + chess_ai_abs(rank - 3);
        score += (7 - center_distance) * chess_ai_weights.endgame_king_activity;
    }

    return score;
}

static int chess_ai_evaluate_opening_principles(const ChessBoard* board, ChessColor color, int phase) {
    int score = 0;
    int home_rank = color == CHESS_COLOR_WHITE ? 0 : 7;
    int queen_home = chess_board_square(3, home_rank);
    int king_home = chess_board_square(4, home_rank);
    int knight_b = chess_board_square(1, home_rank);
    int knight_g = chess_board_square(6, home_rank);
    int bishop_c = chess_board_square(2, home_rank);
    int bishop_f = chess_board_square(5, home_rank);
    int centers[4] = {
        chess_board_square(3, 3),
        chess_board_square(4, 3),
        chess_board_square(3, 4),
        chess_board_square(4, 4)
    };

    if (phase < 14) {
        return 0;
    }

    if (chess_board_get_piece(board, knight_b).type != CHESS_PIECE_KNIGHT) {
        score += chess_ai_weights.develop_knight;
    }
    if (chess_board_get_piece(board, knight_g).type != CHESS_PIECE_KNIGHT) {
        score += chess_ai_weights.develop_knight;
    }
    if (chess_board_get_piece(board, bishop_c).type != CHESS_PIECE_BISHOP) {
        score += chess_ai_weights.develop_bishop;
    }
    if (chess_board_get_piece(board, bishop_f).type != CHESS_PIECE_BISHOP) {
        score += chess_ai_weights.develop_bishop;
    }
    if (chess_board_get_piece(board, king_home).type != CHESS_PIECE_KING) {
        score += chess_ai_weights.castled_king;
    }
    if (chess_board_get_piece(board, queen_home).type != CHESS_PIECE_QUEEN) {
        int developed_minors = 0;
        if (chess_board_get_piece(board, knight_b).type != CHESS_PIECE_KNIGHT) {
            developed_minors += 1;
        }
        if (chess_board_get_piece(board, knight_g).type != CHESS_PIECE_KNIGHT) {
            developed_minors += 1;
        }
        if (chess_board_get_piece(board, bishop_c).type != CHESS_PIECE_BISHOP) {
            developed_minors += 1;
        }
        if (chess_board_get_piece(board, bishop_f).type != CHESS_PIECE_BISHOP) {
            developed_minors += 1;
        }
        if (developed_minors < 2) {
            score -= chess_ai_weights.early_queen_penalty;
        }
    }

    for (int index = 0; index < 4; ++index) {
        ChessPiece piece = chess_board_get_piece(board, centers[index]);
        if (piece.type != CHESS_PIECE_NONE && piece.color == color) {
            score += chess_ai_weights.center_occupy;
        }
        if (chess_is_square_attacked(board, centers[index], color)) {
            score += chess_ai_weights.center_control;
        }
    }

    return score;
}

static int chess_ai_evaluate_for_color(const ChessBoard* board, ChessColor color) {
    ChessColor enemy = chess_color_opposite(color);
    bool endgame = chess_ai_is_endgame(board);
    int phase = chess_ai_game_phase(board);
    int score = 0;
    int bishop_count = 0;
    int enemy_bishop_count = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int value;

        if (piece.type == CHESS_PIECE_NONE) {
            continue;
        }

        if (piece.type == CHESS_PIECE_KING) {
            int mid = chess_ai_king_mid_table[chess_ai_table_square(square, piece.color)];
            int end = chess_ai_king_end_table[chess_ai_table_square(square, piece.color)];
            value = chess_ai_piece_value(piece.type) + (mid * phase + end * (24 - phase)) / 24;
        } else {
            value = chess_ai_piece_value(piece.type)
                    + chess_ai_piece_square_value(piece.type, square, piece.color, endgame);
        }
        if (piece.color == color) {
            score += value;
            if (piece.type == CHESS_PIECE_BISHOP) {
                bishop_count += 1;
            }
        } else {
            score -= value;
            if (piece.type == CHESS_PIECE_BISHOP) {
                enemy_bishop_count += 1;
            }
        }
    }

    if (bishop_count >= 2) {
        score += 35;
    }
    if (enemy_bishop_count >= 2) {
        score -= 35;
    }

    score += chess_ai_evaluate_pawns(board, color);
    score -= chess_ai_evaluate_pawns(board, enemy);
    score += chess_ai_evaluate_rooks(board, color);
    score -= chess_ai_evaluate_rooks(board, enemy);
    score += chess_ai_evaluate_major_piece_safety(board, color);
    score -= chess_ai_evaluate_major_piece_safety(board, enemy);
    score += chess_ai_evaluate_piece_safety(board, color);
    score -= chess_ai_evaluate_piece_safety(board, enemy);
    score += chess_ai_evaluate_king_safety(board, color, phase);
    score -= chess_ai_evaluate_king_safety(board, enemy, phase);
    score += chess_ai_evaluate_opening_principles(board, color, phase);
    score -= chess_ai_evaluate_opening_principles(board, enemy, phase);
    score += chess_ai_evaluate_mating_net(board, color);
    score -= chess_ai_evaluate_mating_net(board, enemy);
    score += (chess_ai_count_legal_moves_for(board, color) - chess_ai_count_legal_moves_for(board, enemy)) * chess_ai_weights.mobility;

    if (!endgame) {
        if (chess_is_in_check(board, color)) {
            score -= 45;
        }
        if (chess_is_in_check(board, enemy)) {
            score += 45;
        }
    }

    if (board->halfmove_clock >= chess_ai_weights.draw_pressure_start) {
        int draw_pressure = (board->halfmove_clock - chess_ai_weights.draw_pressure_start + 1) * chess_ai_weights.draw_pressure_step;
        if (score > 250) {
            score -= draw_pressure;
        } else if (score < -250) {
            score += draw_pressure;
        }
    }

    return score;
}

static int chess_ai_score_to_tt(int score, int ply) {
    if (score > CHESS_AI_MATE_BOUND) {
        return score + ply;
    }
    if (score < -CHESS_AI_MATE_BOUND) {
        return score - ply;
    }
    return score;
}

static int chess_ai_score_from_tt(int score, int ply) {
    if (score > CHESS_AI_MATE_BOUND) {
        return score - ply;
    }
    if (score < -CHESS_AI_MATE_BOUND) {
        return score + ply;
    }
    return score;
}

static ChessAiTtEntry* chess_ai_tt_entry(uint64_t key) {
    return &chess_ai_tt[key % (uint64_t)CHESS_AI_TT_ENTRY_COUNT];
}

static int chess_ai_tt_probe(
        ChessAiContext* context,
        const ChessBoard* board,
        int depth,
        int ply,
        int* alpha,
        int* beta,
        int* out_score,
        ChessMove* out_best_move
) {
    uint64_t key;
    ChessAiTtEntry* entry;

    if (context == 0 || !context->config.use_transposition_table) {
        return 0;
    }

    key = chess_board_compute_zobrist_hash(board);
    entry = chess_ai_tt_entry(key);
    context->tt_probes += 1;
    if (entry->key != key || entry->age != context->search_age) {
        return 0;
    }

    context->tt_hits += 1;
    if (chess_ai_move_valid(entry->best_move) && chess_is_move_legal(board, &entry->best_move)) {
        *out_best_move = entry->best_move;
    }
    if (entry->depth < depth) {
        return 0;
    }

    *out_score = chess_ai_score_from_tt(entry->score, ply);
    if (entry->bound == CHESS_TT_BOUND_EXACT) {
        context->tt_cutoffs += 1;
        return 1;
    }
    if (entry->bound == CHESS_TT_BOUND_LOWER && *out_score > *alpha) {
        *alpha = *out_score;
    } else if (entry->bound == CHESS_TT_BOUND_UPPER && *out_score < *beta) {
        *beta = *out_score;
    }
    if (*alpha >= *beta) {
        context->tt_cutoffs += 1;
        return 1;
    }

    return 0;
}

static void chess_ai_tt_store(
        ChessAiContext* context,
        const ChessBoard* board,
        int depth,
        int ply,
        int score,
        ChessAiTtBound bound,
        ChessMove best_move
) {
    uint64_t key;
    ChessAiTtEntry* entry;

    if (context == 0 || !context->config.use_transposition_table) {
        return;
    }

    key = chess_board_compute_zobrist_hash(board);
    entry = chess_ai_tt_entry(key);
    if (entry->key != 0
            && entry->key != key
            && entry->depth > depth
            && entry->age == context->search_age) {
        return;
    }

    entry->key = key;
    entry->score = chess_ai_score_to_tt(score, ply);
    entry->depth = depth;
    entry->bound = (uint8_t)bound;
    entry->age = context->search_age;
    entry->best_move = best_move;
    context->tt_stores += 1;
}

static int chess_ai_time_expired(ChessAiContext* context) {
    if (context == 0 || context->stop_requested || context->deadline_ms == 0) {
        return context != 0 && context->stop_requested;
    }
    if (((context->nodes + context->quiescence_nodes) & 1023ull) != 0ull) {
        return 0;
    }
    if (chess_ai_now_ms() >= context->deadline_ms) {
        context->stop_requested = 1;
    }
    return context->stop_requested;
}

static void chess_ai_remember_cutoff(ChessAiContext* context, const ChessBoard* board, const ChessMove* move, int depth, int ply) {
    ChessPiece moving;
    int bounded_ply;
    int bonus;

    if (context == 0 || !chess_ai_move_is_quiet(board, move)) {
        return;
    }

    bounded_ply = ply < CHESS_AI_MAX_SEARCH_DEPTH ? ply : CHESS_AI_MAX_SEARCH_DEPTH - 1;
    if (!chess_ai_move_equal(move, &context->killer_moves[bounded_ply][0])) {
        context->killer_moves[bounded_ply][1] = context->killer_moves[bounded_ply][0];
        context->killer_moves[bounded_ply][0] = *move;
    }

    moving = chess_board_get_piece(board, move->from);
    bonus = depth * depth * 16;
    if (bonus < 16) {
        bonus = 16;
    }
    context->history_scores[moving.color][move->from][move->to] += bonus;
    if (context->history_scores[moving.color][move->from][move->to] > 200000) {
        context->history_scores[moving.color][move->from][move->to] = 200000;
    }
}

static int chess_ai_quiescence(
        ChessAiContext* context,
        ChessBoard* board,
        int alpha,
        int beta,
        int ply,
        int remaining_depth
) {
    ChessMoveList moves;
    int stand_pat;
    bool in_check;

    context->quiescence_nodes += 1;
    if (chess_ai_time_expired(context)) {
        return chess_ai_evaluate_for_color(board, board->side_to_move);
    }

    chess_generate_legal_moves(board, &moves);
    in_check = chess_is_in_check(board, board->side_to_move);
    if (moves.count == 0) {
        if (in_check) {
            return -CHESS_AI_MATE_SCORE + ply;
        }
        return 0;
    }

    if (!in_check) {
        stand_pat = chess_ai_evaluate_for_color(board, board->side_to_move);
        if (stand_pat >= beta) {
            return beta;
        }
        if (stand_pat > alpha) {
            alpha = stand_pat;
        }
    }
    if (remaining_depth <= 0) {
        if (in_check) {
            return chess_ai_evaluate_for_color(board, board->side_to_move);
        }
        return alpha;
    }

    chess_ai_order_moves(context, board, &moves, ply, (ChessMove){-1, -1, CHESS_PIECE_NONE});
    for (int index = 0; index < moves.count; ++index) {
        ChessMove move = moves.moves[index];
        ChessBoard copy;
        int score;
        bool gives_check;

        gives_check = chess_ai_move_gives_check(board, &move);
        if (!in_check
                && chess_ai_captured_piece(board, &move).type == CHESS_PIECE_NONE
                && move.promotion == CHESS_PIECE_NONE
                && !gives_check) {
            continue;
        }
        copy = *board;
        if (!chess_apply_move(&copy, &move)) {
            continue;
        }

        score = -chess_ai_quiescence(context, &copy, -beta, -alpha, ply + 1, remaining_depth - 1);
        if (context->stop_requested) {
            return alpha;
        }
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

static int chess_ai_move_extension(const ChessBoard* board, const ChessMove* move, int ply, int previous_to, bool in_check) {
    ChessPiece captured;

    if (board == 0 || move == 0 || ply > 8) {
        return 0;
    }

    captured = chess_ai_captured_piece(board, move);
    if (in_check) {
        return 1;
    }
    if (move->promotion != CHESS_PIECE_NONE || chess_ai_move_gives_check(board, move)) {
        return 1;
    }
    if (previous_to >= 0 && move->to == previous_to && captured.type != CHESS_PIECE_NONE) {
        return 1;
    }
    if (chess_ai_move_is_near_promotion(board, move)) {
        return 1;
    }

    return 0;
}

static int chess_ai_negamax(ChessAiContext* context, ChessBoard* board, int depth, int alpha, int beta, int ply, int previous_to) {
    ChessMoveList moves;
    int best = -CHESS_AI_INF;
    int original_alpha = alpha;
    ChessMove best_move;
    ChessMove tt_move;
    int tt_score = 0;
    bool in_check;

    best_move.from = -1;
    best_move.to = -1;
    best_move.promotion = CHESS_PIECE_NONE;
    tt_move = best_move;

    context->nodes += 1;
    if (chess_ai_time_expired(context)) {
        return chess_ai_evaluate_for_color(board, board->side_to_move);
    }

    chess_generate_legal_moves(board, &moves);
    in_check = chess_is_in_check(board, board->side_to_move);
    if (moves.count == 0) {
        if (in_check) {
            return -CHESS_AI_MATE_SCORE + ply;
        }
        return 0;
    }

    if (depth <= 0) {
        if (context->config.use_quiescence) {
            return chess_ai_quiescence(context, board, alpha, beta, ply, CHESS_AI_MAX_QUIESCENCE_DEPTH);
        }
        return chess_ai_evaluate_for_color(board, board->side_to_move);
    }

    if (chess_ai_tt_probe(context, board, depth, ply, &alpha, &beta, &tt_score, &tt_move)) {
        return tt_score;
    }

    chess_ai_order_moves(context, board, &moves, ply, tt_move);

    for (int index = 0; index < moves.count; ++index) {
        ChessBoard copy = *board;
        int score;
        int extension;
        int child_depth;

        if (!chess_apply_move(&copy, &moves.moves[index])) {
            continue;
        }

        extension = context->config.level == CHESS_AI_HARD
                ? chess_ai_move_extension(board, &moves.moves[index], ply, previous_to, in_check)
                : 0;
        child_depth = depth - 1 + extension;
        if (index == 0 || context->config.level != CHESS_AI_HARD) {
            score = -chess_ai_negamax(context, &copy, child_depth, -beta, -alpha, ply + 1, moves.moves[index].to);
        } else {
            score = -chess_ai_negamax(context, &copy, child_depth, -alpha - 1, -alpha, ply + 1, moves.moves[index].to);
            if (!context->stop_requested && score > alpha && score < beta) {
                score = -chess_ai_negamax(context, &copy, child_depth, -beta, -alpha, ply + 1, moves.moves[index].to);
            }
        }
        if (context->stop_requested) {
            return alpha;
        }
        if (score > best) {
            best = score;
            best_move = moves.moves[index];
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            chess_ai_remember_cutoff(context, board, &moves.moves[index], depth, ply);
            break;
        }
    }

    if (!context->stop_requested) {
        ChessAiTtBound bound = CHESS_TT_BOUND_EXACT;
        if (best <= original_alpha) {
            bound = CHESS_TT_BOUND_UPPER;
        } else if (best >= beta) {
            bound = CHESS_TT_BOUND_LOWER;
        }
        chess_ai_tt_store(context, board, depth, ply, best, bound, best_move);
    }

    return best;
}

static int chess_ai_level_depth(ChessAiLevel level) {
    if (level == CHESS_AI_NORMAL) {
        return 2;
    }
    if (level == CHESS_AI_HARD) {
        return 6;
    }
    return 1;
}

static int chess_ai_level_time_ms(ChessAiLevel level) {
    if (level == CHESS_AI_NORMAL) {
        return 300;
    }
    if (level == CHESS_AI_HARD) {
        return 1800;
    }
    return 40;
}

static ChessAiConfig chess_ai_normalize_config(const ChessAiConfig* config) {
    ChessAiConfig normalized;

    normalized.level = CHESS_AI_NORMAL;
    normalized.max_depth = 0;
    normalized.time_limit_ms = 0;
    normalized.seed = chess_ai_seed;
    normalized.use_quiescence = 1;
    normalized.use_transposition_table = 1;

    if (config != 0) {
        normalized = *config;
    }
    if (normalized.level < CHESS_AI_EASY || normalized.level > CHESS_AI_HARD) {
        normalized.level = CHESS_AI_NORMAL;
    }
    if (normalized.max_depth <= 0) {
        normalized.max_depth = chess_ai_level_depth(normalized.level);
    }
    if (normalized.max_depth > CHESS_AI_MAX_SEARCH_DEPTH) {
        normalized.max_depth = CHESS_AI_MAX_SEARCH_DEPTH;
    }
    if (normalized.time_limit_ms <= 0) {
        normalized.time_limit_ms = chess_ai_level_time_ms(normalized.level);
    }
    if (normalized.use_quiescence < 0) {
        normalized.use_quiescence = 1;
    }
    if (normalized.use_transposition_table < 0) {
        normalized.use_transposition_table = 1;
    }

    return normalized;
}

static void chess_ai_context_init(ChessAiContext* context, const ChessAiConfig* config) {
    memset(context, 0, sizeof(*context));
    context->config = *config;
    context->search_age = chess_ai_tt_age++;
    if (chess_ai_tt_age == 0) {
        chess_ai_tt_age = 1;
    }
    if (config->time_limit_ms > 0) {
        context->deadline_ms = chess_ai_now_ms() + (uint64_t)config->time_limit_ms;
    }
    for (int ply = 0; ply < CHESS_AI_MAX_SEARCH_DEPTH; ++ply) {
        for (int index = 0; index < 2; ++index) {
            context->killer_moves[ply][index].from = -1;
            context->killer_moves[ply][index].to = -1;
            context->killer_moves[ply][index].promotion = CHESS_PIECE_NONE;
        }
    }
}

static void chess_ai_result_clear(ChessAiResult* result) {
    result->best_move.from = -1;
    result->best_move.to = -1;
    result->best_move.promotion = CHESS_PIECE_NONE;
    result->score = 0;
    result->depth_reached = 0;
    result->nodes_searched = 0;
    result->quiescence_nodes = 0;
    result->tt_hits = 0;
    result->tt_cutoffs = 0;
    result->timed_out = 0;
}

static void chess_ai_debug_summary(const ChessAiResult* result, uint64_t elapsed_ms) {
#if CAT_CHESS_AI_DEBUG
    char move[8];

    move[0] = (char)('a' + chess_board_file(result->best_move.from));
    move[1] = (char)('1' + chess_board_rank(result->best_move.from));
    move[2] = (char)('a' + chess_board_file(result->best_move.to));
    move[3] = (char)('1' + chess_board_rank(result->best_move.to));
    move[4] = '\0';
#if defined(__ANDROID__)
    __android_log_print(
            ANDROID_LOG_INFO,
            "CatChess",
            "AI depth=%d score=%d nodes=%llu qnodes=%llu ttHits=%llu ttCutoffs=%llu time=%llums move=%s",
            result->depth_reached,
            result->score,
            (unsigned long long)result->nodes_searched,
            (unsigned long long)result->quiescence_nodes,
            (unsigned long long)result->tt_hits,
            (unsigned long long)result->tt_cutoffs,
            (unsigned long long)elapsed_ms,
            move
    );
#else
    printf(
            "AI depth=%d score=%d nodes=%llu qnodes=%llu ttHits=%llu ttCutoffs=%llu time=%llums move=%s\n",
            result->depth_reached,
            result->score,
            (unsigned long long)result->nodes_searched,
            (unsigned long long)result->quiescence_nodes,
            (unsigned long long)result->tt_hits,
            (unsigned long long)result->tt_cutoffs,
            (unsigned long long)elapsed_ms,
            move
    );
#endif
#else
    (void)result;
    (void)elapsed_ms;
#endif
}

static int chess_ai_easy_move_score(const ChessBoard* board, const ChessMove* move, uint32_t* seed) {
    ChessBoard copy;
    ChessPiece moving;
    ChessPiece captured;
    ChessPiece landed;
    ChessColor enemy;
    int score;

    if (board == 0 || move == 0) {
        return -CHESS_AI_INF;
    }

    moving = chess_board_get_piece(board, move->from);
    captured = chess_ai_captured_piece(board, move);
    score = (int)(chess_ai_next_random(seed) % 41u) - 20;

    if (captured.type != CHESS_PIECE_NONE) {
        score += chess_ai_piece_value(captured.type) - chess_ai_piece_value(moving.type) / 6;
    }
    if (move->promotion != CHESS_PIECE_NONE) {
        score += chess_ai_piece_value(move->promotion) - chess_ai_piece_value(CHESS_PIECE_PAWN);
    }
    if (chess_ai_move_gives_checkmate(board, move)) {
        score += 20000;
    }

    copy = *board;
    if (!chess_apply_move(&copy, move)) {
        return -CHESS_AI_INF;
    }

    enemy = chess_color_opposite(moving.color);
    landed = chess_board_get_piece(&copy, move->to);
    if (landed.type != CHESS_PIECE_NONE && chess_is_square_attacked(&copy, move->to, enemy)) {
        int landed_value = chess_ai_piece_value(landed.type);
        int captured_value = chess_ai_piece_value(captured.type);
        int danger = landed_value - captured_value;

        if (danger > 0) {
            score -= danger;
        }
        if (landed.type == CHESS_PIECE_QUEEN && captured.type == CHESS_PIECE_NONE) {
            score -= 700;
        }
    }

    return score;
}

static ChessMove chess_ai_final_blunder_check(
        const ChessBoard* board,
        ChessMove selected,
        ChessAiDifficulty level
) {
    ChessMoveList moves;
    ChessMove mate_move;
    ChessMove best_safe = selected;
    ChessColor mover;
    int selected_is_legal;
    int best_safe_score = -CHESS_AI_INF;
    int selected_score = -CHESS_AI_INF;
    int winning_score;

    if (board == 0 || level != CHESS_AI_HARD) {
        return selected;
    }

    mover = board->side_to_move;
    if (chess_ai_find_mate_in_one(board, mover, &mate_move)) {
        return mate_move;
    }

    chess_generate_legal_moves(board, &moves);
    selected_is_legal = chess_is_move_legal(board, &selected);
    winning_score = chess_ai_evaluate_for_color(board, mover);

    for (int index = 0; index < moves.count; ++index) {
        ChessBoard copy = *board;
        ChessMove move = moves.moves[index];
        int score;
        int status;

        if (!chess_apply_move(&copy, &move)) {
            continue;
        }
        if (chess_ai_has_mate_in_one(&copy, copy.side_to_move)) {
            continue;
        }

        status = chess_get_game_status(&copy);
        if (status == CHESS_GAME_STALEMATE && winning_score > 450) {
            continue;
        }

        score = chess_ai_evaluate_for_color(&copy, mover);
        if (chess_ai_move_creates_mate_threat(board, &move)) {
            score += 120;
        }
        if (chess_ai_static_exchange_eval(board, &move) < -250) {
            score -= 120;
        }
        if (score > best_safe_score) {
            best_safe_score = score;
            best_safe = move;
        }
        if (selected_is_legal && chess_ai_move_equal(&move, &selected)) {
            selected_score = score;
        }
    }

    if (selected_is_legal && selected_score == -CHESS_AI_INF) {
        ChessBoard selected_copy = *board;
        if (chess_apply_move(&selected_copy, &selected)) {
            selected_score = chess_ai_evaluate_for_color(&selected_copy, mover);
        }
    }

    if (!selected_is_legal) {
        return best_safe;
    }
    if (chess_ai_move_allows_mate_in_one(board, &selected) && best_safe_score > -CHESS_AI_INF) {
        return best_safe;
    }
    if (best_safe_score > selected_score + 250) {
        return best_safe;
    }

    return selected;
}

ChessAiResult chess_ai_find_best_move(const ChessBoard* board, const ChessAiConfig* config) {
    ChessAiConfig normalized = chess_ai_normalize_config(config);
    ChessAiResult result;
    ChessMoveList moves;
    ChessAiContext context;
    uint32_t seed = normalized.seed;
    int best_index = 0;
    int root_scores[256];
    ChessMove root_score_moves[256];
    int root_scores_valid = 0;
    uint64_t start_ms = chess_ai_now_ms();

    chess_ai_result_clear(&result);

    if (board == 0) {
        return result;
    }

    chess_generate_legal_moves(board, &moves);
    if (moves.count <= 0) {
        ChessBoard copy = *board;
        chess_ai_context_init(&context, &normalized);
        result.score = chess_ai_negamax(&context, &copy, 0, -CHESS_AI_INF, CHESS_AI_INF, 0, -1);
        result.nodes_searched = context.nodes;
        result.quiescence_nodes = context.quiescence_nodes;
        result.tt_hits = context.tt_hits;
        result.tt_cutoffs = context.tt_cutoffs;
        return result;
    }

    if (normalized.level == CHESS_AI_EASY) {
        int move_scores[256];
        int best_score = -CHESS_AI_INF;
        int candidate_indexes[256];
        int candidate_count = 0;
        const int near_equal_margin = 90;

        for (int index = 0; index < moves.count; ++index) {
            move_scores[index] = chess_ai_easy_move_score(board, &moves.moves[index], &seed);
            if (move_scores[index] > best_score) {
                best_score = move_scores[index];
            }
        }
        for (int index = 0; index < moves.count; ++index) {
            if (move_scores[index] >= best_score - near_equal_margin && candidate_count < (int)(sizeof(candidate_indexes) / sizeof(candidate_indexes[0]))) {
                candidate_indexes[candidate_count++] = index;
            }
        }
        if (candidate_count > 0) {
            best_index = candidate_indexes[(int)(chess_ai_next_random(&seed) % (uint32_t)candidate_count)];
        }

        result.best_move = moves.moves[best_index];
        result.depth_reached = 1;
        result.nodes_searched = 1;
        result.score = best_score;
        return result;
    }

    chess_ai_context_init(&context, &normalized);
    chess_ai_order_moves(&context, board, &moves, 0, (ChessMove){-1, -1, CHESS_PIECE_NONE});
    result.best_move = moves.moves[best_index];
    result.score = chess_ai_evaluate_for_color(board, board->side_to_move);
    for (int index = 0; index < moves.count; ++index) {
        root_scores[index] = -CHESS_AI_INF;
        root_score_moves[index] = moves.moves[index];
    }

    for (int depth = 1; depth <= normalized.max_depth; ++depth) {
        ChessMove depth_best_move = result.best_move;
        int depth_best_score = -CHESS_AI_INF;
        int alpha = -CHESS_AI_INF;
        int completed = 1;
        int depth_scores[256];

        chess_ai_order_moves(&context, board, &moves, 0, result.best_move);
        for (int index = 0; index < moves.count; ++index) {
            depth_scores[index] = -CHESS_AI_INF;
        }
        for (int index = 0; index < moves.count; ++index) {
            ChessBoard copy = *board;
            int score;

            if (!chess_apply_move(&copy, &moves.moves[index])) {
                continue;
            }

            score = -chess_ai_negamax(&context, &copy, depth - 1, -CHESS_AI_INF, CHESS_AI_INF, 1, moves.moves[index].to);
            if (context.stop_requested) {
                completed = 0;
                break;
            }
            depth_scores[index] = score;
            if (score > depth_best_score) {
                depth_best_score = score;
                depth_best_move = moves.moves[index];
            }
            if (score > alpha) {
                alpha = score;
            }
        }
        if (!completed) {
            result.timed_out = 1;
            break;
        }
        result.best_move = depth_best_move;
        result.score = depth_best_score;
        result.depth_reached = depth;
        for (int index = 0; index < moves.count; ++index) {
            root_scores[index] = depth_scores[index];
            root_score_moves[index] = moves.moves[index];
        }
        root_scores_valid = 1;
    }

    if (root_scores_valid && result.depth_reached > 0 && moves.count > 1 && chess_ai_abs(result.score) < CHESS_AI_MATE_BOUND) {
        int candidate_indexes[256];
        int candidate_count = 0;
        int margin = normalized.level == CHESS_AI_NORMAL ? 35 : 4;
        for (int index = 0; index < moves.count; ++index) {
            if (root_scores[index] >= result.score - margin
                    && candidate_count < (int)(sizeof(candidate_indexes) / sizeof(candidate_indexes[0]))) {
                candidate_indexes[candidate_count++] = index;
            }
        }
        if (candidate_count > 1) {
            result.best_move = root_score_moves[candidate_indexes[(int)(chess_ai_next_random(&seed) % (uint32_t)candidate_count)]];
        }
    }

    result.best_move = chess_ai_final_blunder_check(board, result.best_move, normalized.level);
    if (!chess_is_move_legal(board, &result.best_move)) {
        result.best_move = moves.moves[0];
    }
    result.nodes_searched = context.nodes;
    result.quiescence_nodes = context.quiescence_nodes;
    result.tt_hits = context.tt_hits;
    result.tt_cutoffs = context.tt_cutoffs;
    result.timed_out = result.timed_out || context.stop_requested;
    chess_ai_debug_summary(&result, chess_ai_now_ms() - start_ms);
    return result;
}

int chess_ai_build_principal_variation(const ChessBoard* board, ChessMove* out_moves, int max_moves) {
    ChessBoard copy;
    int count = 0;

    if (board == 0 || out_moves == 0 || max_moves <= 0) {
        return 0;
    }

    copy = *board;
    while (count < max_moves) {
        uint64_t key = chess_board_compute_zobrist_hash(&copy);
        ChessAiTtEntry* entry = chess_ai_tt_entry(key);
        ChessMove move;

        if (entry == 0 || entry->key != key || !chess_ai_move_valid(entry->best_move)) {
            break;
        }
        move = entry->best_move;
        if (!chess_is_move_legal(&copy, &move)) {
            break;
        }
        out_moves[count++] = move;
        if (!chess_apply_move(&copy, &move)) {
            break;
        }
    }

    return count;
}

bool chess_ai_choose_move(const ChessBoard* board, ChessAiDifficulty difficulty, ChessMove* out_move) {
    ChessAiConfig config;
    ChessAiResult result;

    if (board == 0 || out_move == 0) {
        return false;
    }

    config.level = difficulty;
    config.max_depth = 0;
    config.time_limit_ms = 0;
    config.seed = chess_ai_seed;
    config.use_quiescence = 1;
    config.use_transposition_table = 1;

    result = chess_ai_find_best_move(board, &config);
    if (result.best_move.from < 0 || result.best_move.to < 0) {
        return false;
    }

    chess_ai_seed = config.seed * 1103515245u + 12345u;
    *out_move = result.best_move;
    return chess_is_move_legal(board, out_move);
}

int chess_ai_pick_move(const ChessBoard* board, ChessMove* out_move) {
    return chess_ai_choose_move(board, CHESS_AI_NORMAL, out_move) ? 1 : 0;
}
