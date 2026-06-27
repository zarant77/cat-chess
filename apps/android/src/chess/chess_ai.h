#ifndef CAT_CHESS_AI_H
#define CAT_CHESS_AI_H

#include <stdbool.h>
#include <stdint.h>

#include "chess_board.h"
#include "chess_move.h"

typedef enum {
    CHESS_AI_EASY,
    CHESS_AI_NORMAL,
    CHESS_AI_HARD
} ChessAiDifficulty;

typedef ChessAiDifficulty ChessAiLevel;

typedef struct {
    ChessAiLevel level;
    int max_depth;
    int time_limit_ms;
    uint32_t seed;
    int use_quiescence;
    int use_transposition_table;
} ChessAiConfig;

typedef struct {
    ChessMove best_move;
    int score;
    int depth_reached;
    uint64_t nodes_searched;
    uint64_t quiescence_nodes;
    uint64_t tt_hits;
    uint64_t tt_cutoffs;
    int timed_out;
} ChessAiResult;

#define CHESS_AI_PRINCIPAL_VARIATION_MAX 16

uint64_t chess_board_compute_zobrist_hash(const ChessBoard* board);
ChessAiResult chess_ai_find_best_move(const ChessBoard* board, const ChessAiConfig* config);
int chess_ai_build_principal_variation(const ChessBoard* board, ChessMove* out_moves, int max_moves);
bool chess_ai_choose_move(const ChessBoard* board, ChessAiDifficulty difficulty, ChessMove* out_move);
int chess_ai_pick_move(const ChessBoard* board, ChessMove* out_move);

#endif
