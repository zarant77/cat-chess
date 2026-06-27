#include "chess_ai_tuning.h"

#include "chess_fen.h"
#include "chess_move.h"
#include "chess_rules.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef CAT_CHESS_AI_TUNING
#define CAT_CHESS_AI_TUNING 0
#endif

#ifndef CAT_CHESS_AI_TUNING_LOG
#define CAT_CHESS_AI_TUNING_LOG CAT_CHESS_AI_TUNING
#endif

#define AI_TUNING_PV_MAX 12
#define AI_TUNING_MAX_EXPECTED 4
#define AI_TUNING_SELF_PLAY_MAX_PLIES 160

typedef struct {
    const char* name;
    const char* fen;
    const char* expected[AI_TUNING_MAX_EXPECTED];
    ChessAiDifficulty difficulty;
    int max_depth;
    int time_limit_ms;
} ChessAiTacticalCase;

static long ai_tuning_now_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long)now.tv_sec * 1000L + (long)now.tv_nsec / 1000000L;
}

static const char* ai_tuning_difficulty_name(ChessAiDifficulty difficulty) {
    if (difficulty == CHESS_AI_EASY) {
        return "EASY";
    }
    if (difficulty == CHESS_AI_HARD) {
        return "HARD";
    }
    return "NORMAL";
}

static void ai_tuning_move_to_text(ChessMove move, char* buffer, int buffer_size) {
    chess_move_to_uci(&move, buffer, buffer_size);
    if (buffer != 0 && buffer_size > 0 && buffer[0] == '\0') {
        snprintf(buffer, (size_t)buffer_size, "none");
    }
}

static int ai_tuning_move_matches(ChessMove move, const char* const* expected) {
    char uci[8];

    ai_tuning_move_to_text(move, uci, sizeof(uci));
    for (int index = 0; index < AI_TUNING_MAX_EXPECTED; ++index) {
        if (expected[index] == 0 || expected[index][0] == '\0') {
            continue;
        }
        if (strcmp(uci, expected[index]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void ai_tuning_format_pv(const ChessBoard* board, char* buffer, int buffer_size) {
    ChessMove pv[AI_TUNING_PV_MAX];
    int count;
    int offset = 0;

    if (buffer == 0 || buffer_size <= 0) {
        return;
    }
    buffer[0] = '\0';

    count = chess_ai_build_principal_variation(board, pv, AI_TUNING_PV_MAX);
    for (int index = 0; index < count; ++index) {
        char move_text[8];
        int written;

        ai_tuning_move_to_text(pv[index], move_text, sizeof(move_text));
        written = snprintf(
                buffer + offset,
                (size_t)(buffer_size - offset),
                "%s%s",
                index == 0 ? "" : " ",
                move_text
        );
        if (written <= 0 || written >= buffer_size - offset) {
            break;
        }
        offset += written;
    }
}

static ChessAiResult ai_tuning_run_search(
        const ChessBoard* board,
        ChessAiDifficulty difficulty,
        int max_depth,
        int time_limit_ms
) {
    ChessAiConfig config;

    config.level = difficulty;
    config.max_depth = max_depth;
    config.time_limit_ms = time_limit_ms;
    config.seed = 0x12345678u;
    config.use_quiescence = 1;
    config.use_transposition_table = 1;
    return chess_ai_find_best_move(board, &config);
}

static int ai_tuning_run_case(const ChessAiTacticalCase* test_case) {
    ChessBoard board;
    ChessAiResult result;
    long started_ms;
    long elapsed_ms;
    char move_text[8];
    char pv_text[128];
    int passed;

    if (test_case == 0 || !chess_board_from_fen(&board, test_case->fen)) {
#if CAT_CHESS_AI_TUNING_LOG
        printf("AI-TUNE invalid-fen name=\"%s\"\n", test_case != 0 ? test_case->name : "(null)");
#endif
        return 0;
    }

    started_ms = ai_tuning_now_ms();
    result = ai_tuning_run_search(
            &board,
            test_case->difficulty,
            test_case->max_depth,
            test_case->time_limit_ms
    );
    elapsed_ms = ai_tuning_now_ms() - started_ms;
    ai_tuning_move_to_text(result.best_move, move_text, sizeof(move_text));
    ai_tuning_format_pv(&board, pv_text, sizeof(pv_text));
    passed = ai_tuning_move_matches(result.best_move, test_case->expected);

#if CAT_CHESS_AI_TUNING_LOG
    printf(
            "AI-TUNE tactical name=\"%s\" difficulty=%s move=%s score=%d depth=%d nodes=%llu qnodes=%llu time=%ldms pv=\"%s\" result=%s\n",
            test_case->name,
            ai_tuning_difficulty_name(test_case->difficulty),
            move_text,
            result.score,
            result.depth_reached,
            (unsigned long long)result.nodes_searched,
            (unsigned long long)result.quiescence_nodes,
            elapsed_ms,
            pv_text,
            passed ? "PASS" : "FAIL"
    );
#else
    (void)elapsed_ms;
    (void)move_text;
    (void)pv_text;
#endif

    return passed;
}

ChessAiTuningSummary chess_ai_tuning_run_tactical_suite(void) {
    static const ChessAiTacticalCase cases[] = {
        {
            "mate in 1",
            "7k/6pp/8/8/8/8/6PP/5RK1 w - - 0 1",
            {"f1f8", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            800
        },
        {
            "mate in 2",
            "6k1/6pp/8/8/8/8/5PPP/5RK1 w - - 0 1",
            {"f1f8", 0, 0, 0},
            CHESS_AI_HARD,
            5,
            1400
        },
        {
            "win queen",
            "4k3/8/8/8/3q4/8/4N3/4K3 w - - 0 1",
            {"e2d4", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            900
        },
        {
            "avoid mate in 1",
            "6k1/6pp/8/8/8/8/6PP/5RK1 b - - 0 1",
            {"g8f8", "g7g6", 0, 0},
            CHESS_AI_HARD,
            4,
            1000
        },
        {
            "defend hanging queen",
            "4k3/8/8/8/4r3/8/4Q3/4K3 w - - 0 1",
            {"e2e4", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            900
        },
        {
            "fork tactic",
            "6k1/8/8/8/3q4/8/4N3/4K3 w - - 0 1",
            {"e2d4", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            900
        },
        {
            "pinned piece tactic",
            "4r1k1/8/8/8/8/8/4Q3/4K3 w - - 0 1",
            {"e2e8", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            900
        },
        {
            "promotion tactic",
            "6k1/P7/8/8/8/8/8/6K1 w - - 0 1",
            {"a7a8q", 0, 0, 0},
            CHESS_AI_HARD,
            4,
            900
        },
        {
            "avoid stalemate while winning",
            "7k/5Q2/7K/8/8/8/8/8 w - - 0 1",
            {"f7g7", "f7f8", 0, 0},
            CHESS_AI_HARD,
            4,
            900
        }
    };
    ChessAiTuningSummary summary = {0, 0, 0};

    for (int index = 0; index < (int)(sizeof(cases) / sizeof(cases[0])); ++index) {
        int passed = ai_tuning_run_case(&cases[index]);
        summary.total += 1;
        if (passed) {
            summary.passed += 1;
        } else {
            summary.failed += 1;
        }
    }

#if CAT_CHESS_AI_TUNING_LOG
    printf(
            "AI-TUNE tactical-summary passed=%d failed=%d total=%d\n",
            summary.passed,
            summary.failed,
            summary.total
    );
#endif

    return summary;
}

static int ai_tuning_material_score(const ChessBoard* board, ChessColor color) {
    int score = 0;

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        int value = 0;

        if (piece.type == CHESS_PIECE_PAWN) {
            value = 100;
        } else if (piece.type == CHESS_PIECE_KNIGHT) {
            value = 320;
        } else if (piece.type == CHESS_PIECE_BISHOP) {
            value = 330;
        } else if (piece.type == CHESS_PIECE_ROOK) {
            value = 500;
        } else if (piece.type == CHESS_PIECE_QUEEN) {
            value = 900;
        }

        if (piece.type == CHESS_PIECE_NONE || value == 0) {
            continue;
        }
        score += piece.color == color ? value : -value;
    }

    return score;
}

static int ai_tuning_play_game(ChessAiDifficulty white, ChessAiDifficulty black) {
    ChessBoard board;
    ChessGameStatus status = CHESS_GAME_ONGOING;

    chess_board_set_start_position(&board);
    for (int ply = 0; ply < AI_TUNING_SELF_PLAY_MAX_PLIES; ++ply) {
        ChessAiDifficulty difficulty = board.side_to_move == CHESS_COLOR_WHITE ? white : black;
        ChessAiResult result = ai_tuning_run_search(
                &board,
                difficulty,
                difficulty == CHESS_AI_HARD ? 6 : (difficulty == CHESS_AI_NORMAL ? 2 : 1),
                difficulty == CHESS_AI_HARD ? 1200 : (difficulty == CHESS_AI_NORMAL ? 250 : 40)
        );
        if (!chess_is_move_legal(&board, &result.best_move) || !chess_apply_move(&board, &result.best_move)) {
            break;
        }

        status = chess_get_game_status(&board);
        if (status != CHESS_GAME_ONGOING) {
            break;
        }
    }

    if (status == CHESS_GAME_CHECKMATE) {
        return board.side_to_move == CHESS_COLOR_WHITE ? -1 : 1;
    }
    return ai_tuning_material_score(&board, CHESS_COLOR_WHITE);
}

ChessAiTuningSummary chess_ai_tuning_run_self_play_benchmark(void) {
    ChessAiTuningSummary summary = {0, 0, 0};
    int hard_vs_normal;
    int hard_vs_easy;

    hard_vs_normal = ai_tuning_play_game(CHESS_AI_HARD, CHESS_AI_NORMAL);
    hard_vs_easy = ai_tuning_play_game(CHESS_AI_HARD, CHESS_AI_EASY);
    summary.total = 2;
    summary.passed = (hard_vs_normal >= 0 ? 1 : 0) + (hard_vs_easy >= 0 ? 1 : 0);
    summary.failed = summary.total - summary.passed;

#if CAT_CHESS_AI_TUNING_LOG
    printf(
            "AI-TUNE self-play hard_vs_normal=%d hard_vs_easy=%d passed=%d failed=%d total=%d\n",
            hard_vs_normal,
            hard_vs_easy,
            summary.passed,
            summary.failed,
            summary.total
    );
#endif

    return summary;
}

ChessAiTuningSummary chess_ai_tuning_run_all(void) {
    ChessAiTuningSummary tactical = chess_ai_tuning_run_tactical_suite();
    ChessAiTuningSummary self_play = chess_ai_tuning_run_self_play_benchmark();
    ChessAiTuningSummary total;

    total.total = tactical.total + self_play.total;
    total.passed = tactical.passed + self_play.passed;
    total.failed = tactical.failed + self_play.failed;

#if CAT_CHESS_AI_TUNING_LOG
    printf(
            "AI-TUNE all-summary passed=%d failed=%d total=%d\n",
            total.passed,
            total.failed,
            total.total
    );
#endif

    return total;
}

#if CAT_CHESS_AI_TUNING_MAIN
int main(void) {
    ChessAiTuningSummary summary = chess_ai_tuning_run_all();
    return summary.failed == 0 ? 0 : 1;
}
#endif
