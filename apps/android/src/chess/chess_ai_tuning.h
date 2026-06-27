#ifndef CAT_CHESS_AI_TUNING_H
#define CAT_CHESS_AI_TUNING_H

#include "chess_ai.h"

typedef struct {
    int total;
    int passed;
    int failed;
} ChessAiTuningSummary;

ChessAiTuningSummary chess_ai_tuning_run_tactical_suite(void);
ChessAiTuningSummary chess_ai_tuning_run_self_play_benchmark(void);
ChessAiTuningSummary chess_ai_tuning_run_all(void);

#endif
